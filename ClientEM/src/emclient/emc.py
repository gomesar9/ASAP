# -*- coding: utf-8 -*-
import win32file as w
import configparser
import time
import matplotlib.pyplot as plt


class EMClient():

    """Docstring for EMClient. """

    def __init__(self, exec_type="real" , debug=False):
        self.__exec_type = exec_type
        self.__em = None
        self.__setup_ok = False
        self.__pebs_enable = False
        self.__commands = dict()
        self.debug = debug
        self.config = dict()
        self.__hard_config = dict()

        self.load_commands()
        self.load_configuration()


    def __dbg_print(self, msg):
        """TODO: Docstring for __dbg_print.

        """
        if self.debug:
            try:
                print("[D]" + str(msg))
            except Exception as e:
                print("![D] Error: " + str(e))

    def __connect_to_event_monitor(self, mode='rw'):
        """TODO: Docstring for __connect_to_event_monitor.

        """
        if self.__em:
            print("EventMonitor already connected")
            return False

        if mode.lower() == 'r':
            _desiredAccess = w.GENERIC_READ
        elif mode.lower() == 'w':
            _desiredAccess = w.GENERIC_WRITE
        elif mode.lower() == 'rw':
            _desiredAccess = w.GENERIC_WRITE | w.GENERIC_READ
        else:
            print("Invalid mode. Use: mode=['rw'|'r'|'w'].")
            return False

        _shareMode = 0  # Not shared
        _attributes = None
        _creationDisposition = w.OPEN_EXISTING
        _flagsAndAttributes = 0x00000080  # @Marcus's Magic
        _hTemplateFile = None

        try:
            self.__em = w.CreateFile(
                self.__hard_config["driver_full_path"], 
                _desiredAccess,
                _shareMode,
                _attributes,
                _creationDisposition,
                _flagsAndAttributes,
                _hTemplateFile
            )

            return True
        except Exception as e:
            print("[!] '__connect_to_event_monitor()' Error: " + str(e))
            return False


    def __disconnect(self):
        """TODO: Docstring for __disconnect.

        """
        if self.__em:
            w.CloseHandle(self.__em)
            self.__em = None
            return True
        else:
            print("Not connected.")
            return False


    def load_commands(self, commands_file=".cmd.lst"):
        """TODO: Docstring for load_commands.
        :returns: TODO

        """
        with open(commands_file) as cfile:
            for line in cfile.read().splitlines():
                if line:
                    args = line.split("=")
                    self.__commands[args[0]] = args[1]


    def load_configuration(self, config_file="client.cfg"):
        """TODO: Docstring for load_configuration.
        :returns: TODO

        """
        cfgp = configparser.ConfigParser()
        try:
            _ = cfgp.read(config_file)

            self.__hard_config["driver_full_path"] = cfgp.get("EventMonitor", "fpath")
            self.__hard_config["read_bytes"]       = int( cfgp.get("EventMonitor", "read_bytes") )
            self.config["pebs_cores"]       = cfgp.get("EventMonitor", "cores")

            self.config["interval"]         = float( cfgp.get(self.__exec_type, "interval") )
            self.config["n_reads"]          = int( cfgp.get(self.__exec_type, "n_reads") )
            self.config["collector_millis"] = int( cfgp.get(self.__exec_type, "collector_millis") )

            self.config["event"]        = cfgp.get("setup", "event")
            self.config["iteractions"]  = cfgp.get("setup", "iteractions")
            self.config["threshold"]    = cfgp.get("setup", "threshold")

        except Exception as e:
            print("Error: " + str(e))


    def read(self):
        """TODO: Docstring for read.
        :returns: TODO

        """
        try:
            hr, string = w.ReadFile(self.__em, self.__hard_config["read_bytes"], None)
            if hr != 0:
                print("[!] w.ReadFile() error" + str(hr))
            else:
                #TODO properly save data
                return string
            return True
        except Exception as e:
            print("Erro:" + str(e))

        return "-1"


    def write(self, cmd):
        """TODO: Docstring for write.

        :cmd: TODO
        :returns: TODO

        """
        _cmd = self.__cmd_parse(cmd)
        if _cmd:
            try:
                w.WriteFile(self.__em, _cmd.encode(), None)
                return True
            except Exception as e:
                print("Error " + str(e))

        return False



    def __cmd_parse(self, cmd):
        """TODO: Docstring for __cmd_parse.

        :cmd: command to parse
        :returns: message or False

        """
        _msg = ""
        _cmd_list = [x.lower() for x in cmd.split(" ")]

        for i in range( len(_cmd_list) ):
            _c = _cmd_list[i]

            if _c in self.__commands.keys():
                _msg += self.__commands[_c]
            elif _c.isnumeric():
                _msg += _c
            else:
                print("Command '{}' not found and not numeric.".format(_c) )
                return False
            
            if i < len(_cmd_list) - 1:
                _msg += " "

        return _msg

    
    def setup(self):
        """TODO: Docstring for setup.
        :returns: TODO

        """
        # Event
        self.write("{} {}".format(
            self.__commands["set_evt"], 
            self.config["event"]
            ))
        # Iteractions
        self.write("{} {}".format(
            self.__commands["set_itr"], 
            self.config["iteractions"]
            ))
        # Threshold
        self.write("{} {}".format(
            self.__commands["set_thr"], 
            self.config["threshold"]
            ))
        # Collector milliseconds
        self.write("{} {}".format(
            self.__commands["set_cml"], 
            self.config["collector_millis"]
            ))


    def start(self, out_data, plot=False):
        """TODO: Docstring for start.

        :returns: TODO

        """
        if not self.__connect_to_event_monitor():
            return False

        if plot:
            pause_func = plt.pause
        else:
            pause_func = time.sleep

        self.setup()
        # Do the stuff
        count = 0
        print("Starting in 3..")
        time.sleep(3)
        self.write("enable_core1")

        __check = 0
        while count < self.config["n_reads"]:
            data = self.read()
            if data != b'-1':
                print(data)
                out_data.extend( [ int(x) for x in data.decode().split() ] )
                count += 1
                __check = 0

            __check += 1
            if __check > self.config["n_reads"]*20:
                print("EMERGENCY STOP")
                if not self.__disconnect():
                    print("Error in disconnect")
                return False
            pause_func( self.config["interval"] )


        #self.write("disable_core1")

        if not self.__disconnect():
            return False

        return True


    def it_write(self):
        """TODO: Docstring for it_write.
        :returns: TODO

        """
        while True:
            a = input("msg> ")
            if a.lower() in ["q", "exit", "quit"]:
                break
            self.write(a)

