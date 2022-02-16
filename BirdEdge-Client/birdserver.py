#!/usr/bin/env python3

import time
import signal
from zeroconf import ServiceBrowser, Zeroconf, ServiceListener


class BirdclientListener(ServiceListener):
    def add_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        info = zc.get_service_info(type_, name)
        print(f"Discovered {name}: http://{info.server}:{info.port}")

    def update_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        info = zc.get_service_info(type_, name)
        print(f"Updated {name}: http://{info.server}:{info.port}")

    def remove_service(self, zc: Zeroconf, type_: str, name: str) -> None:
        info = zc.get_service_info(type_, name)
        print(f"Removed {name}: http://{info.server}:{info.port}")

    def __init__(self):
        super().__init__()
        self.browser = ServiceBrowser(Zeroconf(), "_birdedge._tcp.local.", self)

        self.running = True
        signal.signal(signal.SIGINT, self.terminate)
        signal.signal(signal.SIGTERM, self.terminate)

        while self.running:
            time.sleep(1)

        self.browser.zc.close()

    def terminate(self, sig, frame):
        self.running = False


if __name__ == "__main__":
    BirdclientListener()
