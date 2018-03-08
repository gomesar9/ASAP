#!/usr/bin/env python
# -*- coding: utf-8 -*-
import win32file as w
import configparser
import time
# plot
import matplotlib.pyplot as plt
from matplotlib import animation
from numpy import arange


def anim(i, cem):
    ydata = cem.last_run_data
    cem.ax.clear()

    #print("[D] len: {}.".format(len(ydata)) )
    xdata = arange(0.0, len(ydata) * cem._ClientEM__interval, cem._ClientEM__interval)
    cem.ax.plot(xdata, ydata)


class ClientEM():
    """docstring for ClientEM"""
    __VERSION = "V0.02a"


    def __init__(self, exec_type="simulation", debug=False):
        print("Client for EventMonitor ({ver}).".format(ver=ClientEM._ClientEM__VERSION))
        self.__exec_type = exec_type

        self.__em = None
        self.__driver_fpath = None
        self.__interval = None
        self.__n_reads = None
        self.__read_bytes = 64
        self.__setup_ok = False

        self.fig = None
        self.ax = None

        self.debug = debug
        self.last_run_data = [0]
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
                self.__driver_fpath, 
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

            self.__driver_fpath = cfgp.get("EventMonitor", "fpath")
            self.__interval = float( cfgp.get(self.__exec_type, "interval") )
            self.__n_reads = int( cfgp.get(self.__exec_type, "n_reads") )

            return self.__connect_to_event_monitor()
        except Exception as e:
            print("[!] '__setup()' Error: " + str(e))
            return False

        return True

    
    def set_config(self, n_reads=None, interval=None):
        if n_reads:
            if type(n_reads) == int:
                self.__n_reads = n_reads
            else:
                print("[!] Invalid 'n_reads' type. Accepted: int")
        if interval:
            if type(interval) == int or type(interval) == float:
                self.__interval = interval
            else:
                print("[!] Invalid 'interval' type. Accepted: int, float")


    def plot_data(self, data_y, data_x=None):
        if not data_x:
            data_x = arange(0.0, len(data_y) * self.__interval, self.__interval)

        fig, ax = plt.subplots()
        ax.plot(data_x, data_y)

        ax.set(
                xlabel="time (s)",
                ylabel="count",
                title="[{}] plot test".format(self.__exec_type)
                )
        ax.grid()

        #fig.savefig("last_run.png")
        plt.show(block=False)


    def start(self):
        if not self.__setup_ok:
            print("[!] Setup failed. Trying again.")
            if not self.__setup():
                return False

        try:
            self.fig = plt.figure()
            self.ax = self.fig.add_subplot(1, 1, 1)
            an = animation.FuncAnimation( self.fig, anim, fargs=(self,) , interval = self.__interval*1000)
            plt.ion()
            plt.show()
            for i in range(self.__n_reads):
                hr, string = w.ReadFile(self.__em, self.__read_bytes, None)
    
                if hr != 0:
                    print("[!] 'start()->w.ReadFile()' Error: " + str(e))
                else:
                    self.last_run_data.append(int(string))
    
                print(".", end='')
                #time.sleep(self.__interval)
                plt.pause(self.__interval)
        
            print("[V] Done.")

            # Plot data
            # Stop!!
            an.event_source.stop()
            an.repeat = False
            plt.ioff()
            plt.show()
    
            # TODO save data
        except Exception as e:
            print("[!] 'start()' Error: " + str(e) )

        return True
 

    def __del__(self):
        w.CloseHandle(self.__em)
        

class CEMShell():
    """Shell for ClientEM"""


    def __init__(self, config_file="client.cfg", debug=False):
        self.debug = debug
        self.config_file = config_file

        self.__cem = ClientEM(debug=self.debug)
        self.__cmds = {
                "run": self.__run,
                "config": self.__config,
                "dbg": self.__set_debug,
                "?": self.__help,
                }


    def __run(self):
        self.__cem.start()


    def __config(self):
        try:
            _n_reads = input("Insert n_reads.\n>")
            _interval = input("Insert interval.\n>")

            self.__cem.set_config(
                    n_reads = int( _n_reads ),
                    interval = float( _interval )
                    )

        except Exception as e:
            print("[!] '__config()' Error: " + str(e))


    def __set_debug(self):
        try:
            _opt = input("CEMS_DEBUG({}). Set debug (on/off):\n>".format(self.debug))
            if _opt == "on":
                self.debug = True
            if _opt == "off":
                self.debug = False
            
            _opt = input("ClientEM_DEBUG({}). Set debug (on/off):\n>".format(self.__cem.debug))
            if _opt == "on":
                self.__cem.debug = True
            if _opt == "off":
                self.__cem.debug = False
        except Exception as e:
            print("[!] '__set_debug()' Error: " + str(e))


    def __help(self):
        print("Commands: run, config, dbg")


    def __exit(self):
        pass


    def menu(self):
        print("[ClientEM Shell]")
        self.__help()
        print("? for help")
        try:
            cmd = None
            while (cmd != "exit"):
                cmd = input("[CEMS]> ")

                if cmd in self.__cmds:
                    if self.debug:
                        print("[D] cmd: '{}'".format(self.__cmds[cmd]))
                    self.__cmds[cmd]()
        except Exception as e:
            print("[!] 'menu()' Error: " + str(e) )

        self.__exit()

        
if __name__ == "__main__":
     #print("Client for EventMonitor ({ver}).".format(ver=ClientEM._ClientEM__VERSION))
     #print("Use:\n c = ClientEM(debug=True)")
     #print(" c.start()")
     #print("# Results in c.last_run_data")
     cems = CEMShell(debug=True)
     cems.menu()


