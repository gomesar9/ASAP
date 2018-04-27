# -*- coding: utf-8 -*-
import win32file as w
import configparser
import time
# plot
import matplotlib.pyplot as plt
from matplotlib import animation
from numpy import arange
import threading


def anim(i, cem):
    ydata = cem.last_run_data
    cem.ax.clear()

    #print("[D] len: {}.".format(len(ydata)) )
    xdata = arange(0.0, len(ydata) * cem._ClientEM__interval, cem._ClientEM__interval)

    # Workaround for not exact calcs
    if len(xdata) == len(ydata):
        cem.ax.plot(xdata, ydata)
    elif len(xdata)+1 == len(ydata):
        cem.ax.plot(xdata[:-1], ydata)


class ClientEM():
    """ Cleint of Event Monitor
        V0.02a: Run simulation, plot graphics
        V0.03a: Enable/disable PEBS with cores 1,2,3 and/or 4.
    """
    __VERSION = "V0.03a"
    __START_CODE = '00'
    __STOP_CODE = '02'


    def __init__(self, exec_type="simulation", debug=False):
        print("Client for EventMonitor ({ver}).".format(ver=ClientEM._ClientEM__VERSION))
        self.__exec_type = exec_type

        self.__em = None
        self.__driver_fpath = None
        self.__interval = None
        self.__n_reads = None
        self.__read_bytes = 64
        self.__setup_ok = False
        self.__pebs_cores = 0
        self.__is_pebs_enable = False

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


    def __connect_to_event_monitor(self, mode='read'):
        if mode.lower() == 'read':
            _desiredAccess = w.GENERIC_READ  # READ/WRITE/EXECUTE
        elif mode.lower() == 'write':
            _desiredAccess = w.GENERIC_WRITE  # READ/WRITE/EXECUTE
        else:
            print("[!] '__connect_to_event_monitor()' mode invalid") 
            return False

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

    def __disconnect_from_event_monitor(self):
        if self.__em:
            w.CloseHandle(self.__em)
            self.__em = None


    def __parse_cores(self, cores):
        r = 0
        for c in cores.split(','):
            r += 2 ** (int(c)-1)

        return '{:02d}'.format(r)


    def __setup(self, cfg_file="client.cfg"):
        cfgp = configparser.ConfigParser()
        try:
            _ = cfgp.read(cfg_file)

            self.__driver_fpath = cfgp.get("EventMonitor", "fpath")
            self.__interval = float( cfgp.get(self.__exec_type, "interval") )
            self.__n_reads = int( cfgp.get(self.__exec_type, "n_reads") )
            self.__pebs_cores = self.__parse_cores( cfgp.get( "EventMonitor", "cores") )

#            return self.__connect_to_event_monitor()
            return True
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


    def enable_pebs(self):
        if not self.__is_pebs_enable:
            if self.__em:
                print("[!] Driver already in use.")
                return False
            if not self.__connect_to_event_monitor(mode='write'):
                return False

            _start_message = '{}{}'.format(ClientEM.__START_CODE, self.__pebs_cores)
            errCod, nBytesWritten = w.WriteFile(self.__em, _start_message.encode(), None)

            self.__disconnect_from_event_monitor()
            if errCod != 0:
                print("Error :" + errCod)
                return False
            else:
                self.__is_pebs_enable = True
                return True
        else:
            print("[!] PEBS is already enabled.")
            return False


    def disable_pebs(self):
        if self.__is_pebs_enable:
            if self.__em:
                print("[!] Driver already in use.")
                return False
            if not self.__connect_to_event_monitor(mode='write'):
                return False
            # TODO: for now disable all cores, but it is possible disable just one
            _stop_message = '{}{}'.format(ClientEM.__STOP_CODE, self.__pebs_cores) 
            errCod, nBytesWritten = w.WriteFile(self.__em, _stop_message.encode(), None)

            self.__disconnect_from_event_monitor()
            if errCod != 0:
                print("Error :" + errCod)
                return False
            else:
                return True
        else:
            print("[!] PEBS is not enabled.")
            return False


    def start(self):
        if not self.__setup_ok:
            print("[!] Setup failed. Trying again.")
            if not self.__setup():
                return False

        try:
            self.__enable_pebs()

            if not __connect_to_event_monitor(mode='read'):
                return False

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
        except Exception as e:
            pass
        self.__disconnect_from_event_monitor()
        self.__disable_pebs()

        return True
 

    def __del__(self):
        self.__disconnect_from_event_monitor()
        

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
                "h": self.__help,
                "help": self.__help
                }


    def __run(self):
        t = threading.Thread(target=self.__cem.start)
        t.start()
        #self.__cem.start()


    def __config(self):
        try:
            print("Insert n_reads (number of read operations). Current: {}".format(
                self.__cem._ClientEM__n_reads )
                )
            _n_reads = input(">")

            print("Insert interval (seconds between reads, can be float). Current: {}".format(
                self.__cem._ClientEM__interval )
                )
            _interval = input(">")

            self.__cem.set_config(
                    n_reads = int( _n_reads ),
                    interval = float( _interval )
                    )

        except Exception as e:
            print("[!] '__config()' Error: " + str(e))


    def __set_debug(self):
        try:
            print("CEMS_DEBUG({}). Set debug (on/off):".format(self.debug))
            _opt = input(">")

            if _opt == "on":
                self.debug = True
            if _opt == "off":
                self.debug = False
            
            print("ClientEM_DEBUG({}). Set debug (on/off):".format(self.__cem.debug))
            _opt = input(">")

            if _opt == "on":
                self.__cem.debug = True
            if _opt == "off":
                self.__cem.debug = False
        except Exception as e:
            print("[!] '__set_debug()' Error: " + str(e))


    def __help(self):
        print("Commands: run, config, dbg, exit|quit, ?|h|help")


    def __exit(self):
        del self.__cem
        print("Exiting")


    def menu(self):
        print("[ClientEM Shell]")
        self.__help()
        print("'?', 'h' or 'help' for help. 'exit' or 'quit' for quit.")
        try:
            cmd = None
            while (cmd != "exit" and cmd != "quit"):
                cmd = input("[CEMS]> ").lower()

                if cmd in self.__cmds:
                    if self.debug:
                        print("[D] cmd: '{}'".format(self.__cmds[cmd]))
                    self.__cmds[cmd]()
        except Exception as e:
            print("[!] 'menu()' Error: " + str(e) )

        self.__exit()

        
if __name__ == "__main__":
     #print("Client for EventMonitor ({ver}).".format(ver=ClientEM._ClientEM__VERSION))
     #print("Use:")
     #print("c = ClientEM(debug=True)")
     #print(" c.start()")
     #print("# Results in c.last_run_data")
     cems = CEMShell(debug=True)
     cems.menu()


