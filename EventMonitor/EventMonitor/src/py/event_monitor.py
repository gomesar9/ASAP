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
            self.__inf_path = cfgp.get("driver", "inf_path")
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
            print("[!] Error:\n" + e.message)
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
            print("[!] Error:\n" + e.message)
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
            self.__driver_name = cfgp.get("device", "path")

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

            print("[D] Connect:\n\t" + cmd)

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
    helptxt =  "# EventMonitor and Client have already been created."
    helptxt += "\n# em = EventMonitor(debug=True)"
    helptxt += "\n# c = Client(debug=True)"
    helptxt += "\n# Use:"
    helptxt += "\nem.install()"
    helptxt += "\nem.start()"
    helptxt += "\n# Connecting Client:"
    helptxt += "\nc.connect()"
    helptxt += "\n# And test:"
    helptxt += "\nc.write('some text')"
    helptxt += "\nc.read()"
    helptxt += "\n# Close Client in the end"
    helptxt += "\nc.close()"
    helptxt += "\n# Stop driver:"
    helptxt += "\nem.stop()"

    print(helptxt)


if __name__ == "__main__":
    em = EventMonitor(debug=True)
    c = Client(debug=True)

    print("For help:\n\t>em_help()")
    
    
