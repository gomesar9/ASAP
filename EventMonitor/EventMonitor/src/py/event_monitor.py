#!/usr/bin/env python
# -*- coding: utf-8 -*-

import win32file as w # Use Windows API
import configparser
import subprocess
import time


class EventMonitor():
    """docstring for EventMonitor"""

    def __init__(self, verbose=False, debug=False):
        self.verbose = verbose
        self.debug = debug

        # Configuration
        self.__driver_name = None
        self.__inf_path = None
        self.__configure()


    def __configure(self, cfg_file="project.cfg"):
        cfgp = configparser.ConfigParser()
        _ = cfgp.read(cfg_file)

        path_to_inf = ''
        try:
            _project_path = cfgp.get("project", "fpath")
            _inf_path = cfgp.get("driver", "inf_rpath")
            self.__inf_path = _project_path + _inf_path

            self.__driver_name = cfgp.get("driver", "name")

            if self.verbose:
                print("[V] Driver configuration [OK].")
        except Exception as e:
            print("[!] Erro: " + str(e))
            return False

        return True


    def __doSetupAction(self, action):
        cmd = "RUNDLL32.EXE SETUPAPI.DLL,InstallHinfSection {ACT} 132 {INF}"
        cmd = cmd.format(ACT=action, INF=self.__inf_path)

        if self.debug:
            print("[D] __doSetupAction")
            print("\tCMD: " + cmd)

        try:
            subprocess.check_call( cmd, shell=True )
            
            if self.verbose:
                print("[V] Driver setup [OK].")
        except Exception as e:
            print("[!] Error: " + str(e) )
            return False

        return True


    def __doScAction(self, action):
        cmd = "sc {ACT} {DRIVER}"
        cmd = cmd.format(ACT=action, DRIVER=self.__driver_name)

        if self.debug:
            print("[D] __doScAction")
            print("\tCMD: " + cmd)

        try:
            s = subprocess.Popen( cmd, shell=True )
            return True
        except Exception as e:
            print("[!] Error: " + e.message)
            return False


    def install(self):
        action = "DefaultInstall"

        return self.__doSetupAction(action)


    def uninstall(self):
        action = "DefaultUninstall"

        return self.__doSetupAction(action)


    def start(self):
        action = "start"

        return self.__doScAction(action)

        
    def stop(self):
        action = "stop"

        return self.__doScAction(action)


    def interrogate(self):
        action = "interrogate"

        return self.__doScAction(action)


class Client():
    def __init__(self, debug=False):
        self.debug = debug
        self.hdvc = None

        # Configure
        self.__driver_name = None
        self.__configure()

        # Connect
        #self.__connect()

    
    def __configure(self, cfg_file="project.cfg"):
        cfgp = configparser.ConfigParser()
        cfgp.read(cfg_file)

        try:
            self.__driver_name = cfgp.get("device", "fpath")

            if self.debug:
                print("[D] Client configure [OK].")
        except Exception as e:
            print("[!] Error: " + str(e))

            return False

        return True


    def connect(self):
        _desiredAccess = w.GENERIC_READ|w.GENERIC_WRITE  # READ/WRITE/EXECUTE
        _shareMode = 0  # Not shared
        _attributes = None
        _creationDisposition = w.OPEN_EXISTING
        _flagsAndAttributes = 0x00000080  # @Marcus's Magic
        _hTemplateFile = None

        if self.debug:
            cmd = "w.CreateFile({}, {}, {}, {}, {}, {}, {})".format(
                self.__driver_name, 
                _desiredAccess,
                _shareMode,
                _attributes,
                _creationDisposition,
                _flagsAndAttributes,
                _hTemplateFile
                )

            print("[D] Connect: '" + cmd + "'")

        self.hdvc = w.CreateFile(
                self.__driver_name, 
                _desiredAccess,
                _shareMode,
                _attributes,
                _creationDisposition,
                _flagsAndAttributes,
                _hTemplateFile
                )
        
        #time.sleep(1)
    
    
    def write(self, text):
        '''

            https://docs.activestate.com/activepython/3.2/pywin32/win32file__WriteFile_meth.html
        '''
        if self.hdvc == None:
            print("Client not connected.")
            return False

        errCod, nBytesWritten = w.WriteFile(self.hdvc, text.encode(), None)
        if self.debug:
            print("[D] Send: errCod({}), nBytesWritten({}).".format(
                errCod, nBytesWritten
                ))

        if errCod != 0:
            print("Error: " + errCod)
            return False
        else:
            return True


    def read(self):
        '''

            hr may be 0, ERROR_MORE_DATA or ERROR_IO_PENDING
            https://docs.activestate.com/activepython/3.2/pywin32/win32file__ReadFile_meth.html
        '''
        if self.hdvc == None:
            print("Client not connected.")
            return False

        hr, string = w.ReadFile(self.hdvc, 128, None)

        if hr != 0:
            print("Error: " + hr)
            return False, ''
        else:
            try:
                string = string.decode()
                return True, string
            except Exception as e:
                print("[!] Erro: " + str(e))
                return False, ''


    def close(self):
        w.CloseHandle(self.hdvc)
        if self.debug:
            print("EMClient closed.")


    def __del__(self):
        print("Garbage __del__")
        self.close()


def em_help():
    print("# EventMonitor and Client have already been created.")
    print("# em = EventMonitor(debug=True)")
    print("# c = Client(debug=True)")
    print("# Use:")
    print("em.install()")
    print("em.start()")
    print("# Connecting Client:")
    print("c.connect()")
    print("# And test:")
    print("c.write('some text')")
    print("c.read()")
    print("# Close Client in the end")
    print("c.close()")
    print("# Stop driver:")
    print("em.stop()")


class EMShell():
    """Shell for Event Monitor"""


    def __init__(self, config_file='project.cfg', debug=False):
        self.debug = debug
        self.tmp = config_file
        self.em = EventMonitor(verbose=True, debug=True)
        self.cl = Client(debug=False)
        self.__cmds = {
                "install": self.__install,
                "start": self.__start,
                "stop": self.__stop,
                "interrogate": self.__interrogate,
                "read": self.__read,
                "write": self.__i_write,
                "?": self.__help
                }
        self.__cl_connected = False
        self.__em_started = False


    def __print(self, txt):
        print("[EMS] " + txt)


    def __help(self):
        print("Commands: install, start, stop, interrogate, read, write, exit")
        print("It is not necessary to run 'start' before 'write'/'read', either 'stop' before 'exit'.")


    def __install(self):
        if self.__cl_connected:
            self.__disconnect()

        if self.__em_started:
            self.__stop()

        self.__print("Installing driver.")
        self.em.install()


    def __start(self):
        if self.__em_started:
            self.__print("Driver already started.")
        else:
            self.__print("Starting driver.")
            self.em.start()
            self.__em_started = True


    def __stop(self):
        if not self.__em_started:
            self.__print("Driver has not started.")
        else:
            self.__print("Stopping driver.")
            self.em.stop()
            self.__em_started = False


    def __interrogate(self):
        if not self.__em_started:
            self.__start()
        self.em.interrogate()

    
    def __connect(self):
        if not self.__em_started:
            self.__start()
    
        time.sleep(5)
        self.__print("Connecting client.")
        self.cl.connect()
        self.__cl_connected = True


    def __disconnect(self):
        if self.__cl_connected:
            self.__print("Disconnecting client.")
            self.cl.close()
            self.__cl_connected = False


    def __i_write(self):
        if not self.__cl_connected:
            self.__connect()

        try:
            msg = input("msg>")
            self.cl.write(msg)
        except Exception as e:
            print(str(e))


    def __read(self):
        if not self.__cl_connected:
            self.__connect()

        _, txt = self.cl.read()
        print("msg>" + txt)


    def menu(self):
        print("[EventMonitorShell]")
        print("[V] ? for help")
        try:
            cmd = None
            while cmd != "exit":
                print("")
                cmd = input("[EMS]> ")

                if cmd in self.__cmds:
                    if self.debug:
                        print("CMD({})".format(cmd))
                    self.__cmds[cmd]()

        except Exception as e:
            print( str(e) )

        self.cl.close()
        time.sleep(3)
        if self.__em_started:
            self.em.stop()


if __name__ == "__main__":
    #em = EventMonitor(debug=True)
    #c = Client(debug=True)

    #print("For help: 'em_help()'")
    ems = EMShell()
    ems.menu()
    
