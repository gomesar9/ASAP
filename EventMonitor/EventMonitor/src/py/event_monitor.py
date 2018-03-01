# -*- coding: utf-8 -*-

import win32file as w # Use Windows API
import subprocess
import time


class EventMonitor():
    """docstring for EventMonitor"""

    def __init__(self, verbose=False, debug=False):
        self.verbose = verbose
        self.debug = debug

        # Hard coded
        self.__DRIVER_NAME = "EventMonitor10"
        #self.__INF_PATH = "C:\\Users\\lasca\\Source\\Repos\\2017-marcus-alexandre\\EventMonitor\\EventMonitor\\x64\\Debug\\EventMonitor10.inf"
        self.__INF_PATH = EventMonitor.getPath()

    def getPath():
        path_to_inf = ''
        with open('inf_path.txt') as f:
            path_to_inf = f.read().splitlines()[0]

        return path_to_inf


    def __doSetupAction(self, action):
        cmd = "RUNDLL32.EXE SETUPAPI.DLL,InstallHinfSection {ACT} 132 {INF}"
        cmd = cmd.format(ACT=action, INF=self.__INF_PATH)

        if self.debug:
            print("[D] __doSetupAction")
            print("\tCMD: " + cmd)

        try:
            subprocess.check_call( cmd, shell=True )
            return True
        except Exception as e:
            print("[!] Error:\n" + e.message)
            return False


    def __doScAction(self, action):
        cmd = "sc {ACT} {DRIVER}"
        cmd = cmd.format(ACT=action, DRIVER=self.__DRIVER_NAME)

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
        self.DRIVER_NAME = "\\\\.\\EventMonitor"
        self.hdvc = None

        self.__setup()


    def __setup(self):
        _desiredAccess = w.GENERIC_READ|w.GENERIC_WRITE  # READ/WRITE/EXECUTE
        _shareMode = 0  # Not shared
        _attributes = None
        _creationDisposition = w.OPEN_EXISTING
        _flagsAndAttributes = 0x00000080  # @Marcus's Magic
        _hTemplateFile = None

        self.hdvc = w.CreateFile(
                self.DRIVER_NAME, 
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
        errCod, nBytesWritten = w.WriteFile(self.hdvc, text.encode(), None)
        if self.debug:
            print("[D]send: errCod({}), nBytesWritten({}).".format(
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
        hr, string = w.ReadFile(self.hdvc, 128, None)

        if hr != 0:
            print("Error: " + hr)
            return False, ''
        else:
            try:
                string = string.decode()
                return True, string
            except Exception as e:
                print("[!] Erro: " e.msg)
                return False, ''


    def close(self):
        w.CloseHandle(self.hdvc)
        if self.debug:
            print("EMClient closed.")


    def __del__(self):
        print("Garbage __del__")
        self.close()


def em_help():
    helptxt = "# em = EventMonitor(debug=True) # EventMonitor was created."
    helptxt += "\n# Use:"
    helptxt += "\nem.install()"
    helptxt += "\nem.start()"
    helptxt += "\n# Creating Client:"
    helptxt += "\nc = Client(verbose=True)"
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
    #c = Client(debug=True)
    print("For help:\n\t>em_help()")
    
    
