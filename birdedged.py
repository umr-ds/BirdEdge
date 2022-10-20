#!/usr/bin/env python3

import argparse
import configparser
import io
import json
import logging
import signal
import subprocess
import time
import threading
import csv
import platform

from zeroconf import ServiceBrowser, Zeroconf, ServiceListener
import paho.mqtt.client

argparser = argparse.ArgumentParser(
    prog='birdedged',
    description='BirdEdge Daemon - A tool to run BirdEdge classification.',
    formatter_class=argparse.ArgumentDefaultsHelpFormatter,
)
argparser.add_argument('-c', '--config-path', help='Path to template config.', default='configs/birdedged.conf')
argparser.add_argument("-v", "--verbose", help="increase output verbosity", action="count", default=0)
argparser.add_argument("--export-path", help="Path to export the current configuration to.", default='configs/dynamic.conf')
argparser.add_argument("--restart-interval", help="Minimal time interval between classification process restarts", default=60, type=int)
argparser.add_argument("--simulate", help="simulate execution using mock data (only for development)", action="store_true")

publish_options = argparser.add_argument_group("publish")
# publish_options.add_argument("--path", help="file output path", default="data", type=str)
# publish_options.add_argument("--csv", help="enable csv data publishing", action="store_true")
# publish_options.add_argument("--archive-config", help="archive configuration running birdedge", action="store_true")
publish_options.add_argument("--mqtt", help="enable mqtt data publishing", action="store_true")
publish_options.add_argument("--mqtt-host", help="hostname of mqtt broker", default="localhost", type=str)
publish_options.add_argument("--mqtt-port", help="port of mqtt broker", default=1883, type=int)
publish_options.add_argument("--mqtt-qos", help="mqtt quality of service level (0, 1, 2)", default=1, type=int)
publish_options.add_argument("--mqtt-keepalive", help="timeout for mqtt connection (s)", default=3600, type=int)
publish_options.add_argument("-mv", "--mqtt-verbose", help="increase mqtt logging verbosity", action="count", default=0)


class MQTTConsumer(logging.StreamHandler):
    def __init__(
        self,
        mqtt_host: str,
        mqtt_port: int,
        mqtt_qos: int,
        mqtt_keepalive: int,
        mqtt_verbose: int,
        prefix: str = "/birdedge",
        **kwargs,
    ):
        logging_level = max(0, logging.WARN - (mqtt_verbose * 10))
        super(logging.StreamHandler, self).__init__(level=logging_level)

        fmt = logging.Formatter("%(message)s")
        self.setFormatter(fmt)

        self.prefix = platform.node() + prefix
        self.mqtt_qos = mqtt_qos
        self.stream = paho.mqtt.client.Client(f"{platform.node()}-birdedge", clean_session=False)
        logging.info("Connecting to MQTT node %s:%s", mqtt_host, mqtt_port)
        self.stream.connect(mqtt_host, mqtt_port, keepalive=mqtt_keepalive)
        self.stream.loop_start()

    def __del__(self):
        logging.info("Stopping MQTT thread")
        self.stream.loop_stop()

    def emit(self, record):
        path = f"{self.prefix}/log"

        # publish csv
        csv_io = io.StringIO()
        csv.writer(csv_io, dialect="excel", delimiter=";").writerow([record.levelname, record.name, self.format(record)])
        payload_csv = csv_io.getvalue().splitlines()[0]
        self.stream.publish(path + "/csv", payload_csv, qos=self.mqtt_qos)

    def add(self, signal: list):
        path = f"{self.prefix}/species"

        # publish csv
        csv_io = io.StringIO()
        csv.writer(csv_io, dialect="excel", delimiter=";").writerow(signal)
        payload_csv = csv_io.getvalue().splitlines()[0]
        logging.debug("publishing via mqtt (%s B): %s", len(payload_csv), payload_csv)
        self.stream.publish(path + "/csv", payload_csv, qos=self.mqtt_qos)


class BirdEdgeDaemon(ServiceListener):
    def __init__(self,
                 config_path,
                 export_path,
                 restart_interval,
                 simulate: bool,
                 mqtt_c: MQTTConsumer,
                 **kwargs,
                 ):

        self.config_path = config_path
        self.export_path = export_path
        self.restart_interval = restart_interval
        self.simulate = simulate

        # read initial config
        self.config = configparser.ConfigParser()
        self.config.read(config_path)

        # setup mqtt consumer
        self.mqtt_c = mqtt_c

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

        # get number of active streams
        active_streams = 0
        for name, section in self.config.items():
            if name.startswith("source"):
                if "1" in section.get("enable"):
                    active_streams += 1

        # adapt batch-sizes
        self.config.set("streammux", "batch-size", str(active_streams))
        self.config.set("audio-classifier", "batch-size", str(active_streams))

        # writing config file
        with open(self.export_path, "wt", encoding="utf-8") as export_file:
            self.config.write(export_file)

    def publish_classification(self, json_data):
        if json_data["label"].strip() in ["", "00_background"]:
            return

        uri = self.config.get(f"source{json_data['source_id']}", "uri")
        station = uri[7:].split(":")[0]

        self.mqtt_c.add([json_data["timestamp"], station, json_data["label"].strip(), json_data['confidence']])

    def run(self):
        self.running = True

        while self.running:
            logging.debug("Writing config and running classification process.")
            self.last_restart_ts = time.time()
            self.write_config()

            if self.simulate:
                self.process = subprocess.Popen(
                    ["bash", "-c", "while read -r LINE; do echo \"$LINE\"; sleep $(($RANDOM % 3)); done < misc/birdedge.stdout"],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                )
            else:
                self.process = subprocess.Popen(
                    ["./birdedge", "-c", self.export_path],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    env={"LD_PRELOAD": "/usr/lib/aarch64-linux-gnu/libgomp.so.1"},
                )

            for line in io.TextIOWrapper(self.process.stdout, encoding="utf-8"):
                line = line[:-1]
                if len(line) == 0:
                    continue

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

    mqtt_c = MQTTConsumer(**args.__dict__)
    logging.getLogger().addHandler(mqtt_c)

    logging.info("Starting BirdEdge Daemon")
    daemon = BirdEdgeDaemon(mqtt_c=mqtt_c, **args.__dict__)

    # allow for 5 seconds for zeroconf to discover services
    time.sleep(5)
    daemon.run()
