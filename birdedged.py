#!/usr/bin/env python3

import configparser
import subprocess
import argparse
import logging
import json
import signal
import time
import threading

from zeroconf import ServiceBrowser, Zeroconf, ServiceListener
from influxdb_client import InfluxDBClient
from influxdb_client.client.write_api import SYNCHRONOUS

argparser = argparse.ArgumentParser(
    prog='birdedged',
    description='BirdEdge Daemon - A tool to run BirdEdge classification.',
    formatter_class=argparse.ArgumentDefaultsHelpFormatter,
)
argparser.add_argument('-c', '--config-path', help='Path to template config.', default='configs/birdedged.conf')
argparser.add_argument("-v", "--verbose", help="increase output verbosity", action="count", default=0)
argparser.add_argument("--export-path", help="Path to export the current configuration to.", default='configs/dynamic.conf')
argparser.add_argument("--restart-interval", help="Minimal time interval between classification process restarts", default=60, type=int)

influx_group = argparser.add_argument_group("InfluxDB")
influx_group.add_argument("--influx-url", help="InfluxDB url", default='http://localhost:9999')
influx_group.add_argument("--influx-token", help="InfluxDB token")
influx_group.add_argument("--influx-org", help="InfluxDB organization", default='default')


class BirdEdgeDaemon(ServiceListener):
    def __init__(self,
                 config_path,
                 export_path,
                 restart_interval,
                 influx_url,
                 influx_token,
                 influx_org,
                 **kwargs,
                 ):

        self.config_path = config_path
        self.export_path = export_path
        self.restart_interval = restart_interval

        # read initial config
        self.config = configparser.ConfigParser()
        self.config.read(config_path)

        # setup influxdb client
        self.influxc = InfluxDBClient(url=influx_url, token=influx_token, org=influx_org)
        self.influxc_write = self.influxc.write_api(write_options=SYNCHRONOUS)

        # set up termination handler
        signal.signal(signal.SIGINT, self.terminate)
        signal.signal(signal.SIGTERM, self.terminate)

        # initialize running process vars
        self.process: subprocess.Popen = None
        self.last_restart_ts = None
        self.running = False

        # initialize zeroconf
        self.browser = ServiceBrowser(Zeroconf(), "_birdedge._tcp.local.", self)

    def get_source(self, client):
        i = 0

        for name, section in self.config.items():
            if name.startswith("source"):
                i += 1
                if client in section.get("uri"):
                    return name

        self.config.add_section(f"source{i}")
        return f"source{i}"

    def restart_classification(self):
        if self.last_restart_ts is None:
            return

        if self.process is None:
            return

        restart_wait_s = self.restart_interval - (time.time() - self.last_restart_ts)
        if restart_wait_s <= 0:
            logging.info("Restart classification process.")
            self.process.terminate()
        else:
            logging.info("Restarting classification process in %d seconds.", restart_wait_s)
            threading.Timer(restart_wait_s, self.restart_classification).start()

    def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        info = zc.get_service_info(type_, name)
        logging.debug("Discovered %s", info)

        # Get source if already exists
        name = self.get_source(info.server)

        # set values of discovered service
        self.config.set(name, "enable", "1")
        self.config.set(name, "type", "7")
        self.config.set(name, "uri", f"http://{info.server}:{info.port}/stream.wav")
        self.config.set(name, "num-sources", "1")
        self.config.set(name, "gpu-id", "1")
        self.config.set(name, "latency", "20")

        # restart running classification process
        self.restart_classification()

    def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        self.add_service(zc, type_, name)

    def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        info = zc.get_service_info(type_, name)
        logging.debug("Disableing %s", info)

        # Get source and disable
        name = self.get_source(info.server)
        self.config.set(name, "enable", "0")

        # restart running classification process
        self.restart_classification()

    def write_config(self):
        logging.debug("Writing config to %s", self.export_path)
        with open(self.export_path, "wt", encoding="utf-8") as export_file:
            self.config.write(export_file)

    def publish_classification(self, json_data):
        uri = self.config.get(f"section{json_data['source_id']}", "uri")
        station = uri[7:].split(":")[0]

        point = {
            "measurement": "birdedge",
            "tags": {
                "station": station,
                "label": json_data['label'],
            },
            "fields": {
                "confidence": json_data['confidence'],
            },
            "time": json_data["timestamp"],
        }

        self.influxc_write.write("birdedge", point)

    def run(self):
        self.running = True

        while self.running:
            logging.debug("Writing config and running classification process.")
            self.last_restart_ts = time.time()
            self.write_config()

            self.process = subprocess.Popen(
                ["./birdedge", "-c", self.export_path],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                env={"LD_PRELOAD": "/usr/lib/aarch64-linux-gnu/libgomp.so.1"},
            )

            for line in iter(self.process.stdout.readline, b""):
                line = line[:-1].decode("utf-8")

                if line.startswith("{"):
                    json_data = json.loads(line)
                    self.publish_classification(json_data)
                    continue

                # add message to respective logging level
                if line.startswith("WARNING: "):
                    logging.warning(line[9:])
                elif line.startswith("ERROR: "):
                    logging.error(line[7:])
                elif line.startswith("INFO: "):
                    logging.info(line[6:])
                else:
                    logging.info(line)

                # disable connection, if
                if line.startswith("Could not connect") or line.startswith("Error resolving"):
                    err = {el.split(":")[0].strip(): ":".join(el.split(":")[1:]).strip()
                           for el in line.split(",")}
                    source = self.get_source(err["URL"])

                    logging.error("Disableing source %s", err["URL"])
                    self.config.set(source, "enable", "0")

                    # restart running classification process
                    self.restart_classification()

    def terminate(self, sig, frame):
        self.running = False
        self.browser.zc.close()
        if self.process:
            self.process.terminate()

        logging.info("Termination complete.")


if __name__ == "__main__":
    args = argparser.parse_args()

    logging_level = logging.WARNING - 10 * args.verbose
    logging.basicConfig(level=logging_level)

    logging.info("Starting BirdEdge Daemon")
    daemon = BirdEdgeDaemon(**args.__dict__)

    # allow for 5 seconds for zeroconf to discover services
    time.sleep(5)
    daemon.run()
