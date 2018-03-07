#!/usr/bin/env python
# -*- coding: utf-8 -*-
import win32file as w
import configparser
import time


class ClientEM():
    """docstring for ClientEM"""
    __VERSION = "V0.01"


    def __init__(self, exec_type="simulation", debug=False):
        self.__interval = None
        self.__n_reads = None
        self.__em = None
        self.__setup_ok = False
        self.__exec_type = exec_type

        self.debug = debug
        self.last_run_data = None
        if not self.__setup():
            print("Please use .__setup(cfg_file='path_to_cfg_file') with a valid configuration file.")
        else:
            self.__setup_ok = True


    def __dbg_print(self, txt):
        if self.debug:
            try:
                print("[D] " + str(txt))
            except Exception as e:
                print("[!][D] ERROR: " + str(e))


    def __connect_to_event_monitor(self):
        _desiredAccess = w.GENERIC_READ  # READ/WRITE/EXECUTE
        _shareMode = 0  # Not shared
        _attributes = None
        _creationDisposition = w.OPEN_EXISTING
        _flagsAndAttributes = 0x00000080  # @Marcus's Magic
        _hTemplateFile = None

        try:
            self.__em = w.CreateFile(
		self.__driver_name, 
		_desiredAccess,
		_shareMode,
		_attributes,
		_creationDisposition,
		_flagsAndAttributes,
		_hTemplateFile
		)
        except Exception as e:
            print("[!] '__connect_to_event_monitor()' Error: " + str(e))
            return False

        return True


    def __setup(self, cfg_file="client.cfg"):
        cfgp = configparser.ConfigParser()
        try:
            _ = cfgp.read(cfg_file)

            self.__interval = float( cfgp.get(self.__exec_type, "interval") )
            self.__n_reads = cfgp.get(self.__exec_type, "n_reads")

            return self.__connect_to_event_monitor()
        except Exception as e:
            print("[!] '__setup()' Error: " + str(e))
            return False

        return True


    def start(self):
        if not self.__setup_ok:
            print("[!] Setup failed. Trying again.")
            if not self.__setup():
                return False


        data = list()
        for i in range(self.__n_reads):
            hr, string = w.ReadFile(self.__em, self.__read_bytes, None)

            if hr != 0:
                print("[!] Error: " + str(e))
            else:
                data.append(string)

            time.sleep(self.__interval)
        
        # TODO plot graphic

        self.last_run_data = data
        # TODO save data

        return True
 

if __name__ == "__main__":
     print("Client for EventMonitor ({ver}).".format(ver=ClientEM._ClientEM__VERSION))

