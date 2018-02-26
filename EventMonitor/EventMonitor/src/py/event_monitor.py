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


class Test():
    """docstring for Test"""
    def __init__(self, verbose=False):
        self.verbose = verbose
        self.DRIVER_NAME = "\\\\.\\EventMonitor"

    
    def run(self, opt='wr'):
        '''
            params:
                opt: 'w'|'r'|'wr'|
                    w - write test
                    r - read test

        '''

        if 'w' in opt:
            if self.verbose:
                print("[V] Conectando ao driver em modo de escrita.")
            drvw = open(self.DRIVER_NAME, 'w')
            
            if self.verbose:
                print("[V] Enviando mensagem.")
            msg = "Hello"
            print("[:] Enviando msg: '" + msg + "'")
            drvw.write(msg)
            drvw.flush()

            if self.verbose:
                print("[V] Fechando conexão.")
            time.sleep(1)
            drvw.close()
            time.sleep(1)

        if 'r' in opt:
            if self.verbose:
                print("[V] Conectando ao driver em modo de leitura.")
            drvr = open(self.DRIVER_NAME, 'r')
            
            if self.verbose:
                print("[V] Lendo mensagem (6 bytes)")
            msg = drvr.read(6)
            if msg:
                print("[:] Msg recebida: " + msg)
            else:
                print("[:] Msg não recebida/vazia.")

            if self.verbose:
                print("[V] Fechando conexão.")
            time.sleep(1)
            drvr.close()


def doPipeline(verbose=False, debug=False):
    '''
        params:
            verbose: True|False
            debug: True|False

        Step01: install driver, start driver
        Step02: run 'write' and 'read' test
        Step03: stop driver
    '''

    em = EventMonitor(verbose=verbose, debug=debug)
    tt = Test(verbose=verbose)

    # -- Step01
    if verbose:
        print("[V] Pipeline #Step01.")
    em.install()
    time.sleep(2)
    em.start()

    # -- Step02
    if verbose:
        print("[V] Pipeline #Step02.")
    time.sleep(2)
    tt.run()

    # -- Step03
    if verbose:
        print("[V] Pipeline #Step03.")
    time.sleep(2)
    em.stop()


if __name__ == "__main__":
    em = EventMonitor(debug=True)
    tt = Test(verbose=True)
    
    
