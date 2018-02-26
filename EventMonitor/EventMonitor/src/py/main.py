import subprocess

DRIVERNAME = "EventMonitor10"
INFO_PATH = "C:\\Users\\lasca\\Source\\Repos\\2017-marcus-alexandre\\EventMonitor\\EventMonitor\\x64\\Debug\\EventMonitor10.inf"
# "C:\\Users\\lasca\\Source\\Repos\\2017-marcus-alexandre\\EventMonitor\\x64\\Debug\\EventMonitor10.inf"
# C:\Users\lasca\Source\Repos\2017-marcus-alexandre\EventMonitor\x64\Debug\EventMonitor10\eventmonitor10.cat
def event_monitor(action="start"):
    '''
        event_monitor function handles the EventMonitor driver.
        
        PARAMS:
            action= "start" | "stop" | "interrogate"
    '''
    action = action.lower()
    cmd = "sc {ACT} " + DRIVERNAME
    supported_actions = ["start", "stop", "interrogate"]
    
    if action in supported_actions:
        s = subprocess.Popen(cmd.format(ACT=action),shell=True)
        
        return True
    else:
        print("Unsupported action. Try:")
        print(supported_actions)
        
        return False


def setup_event_monitor(action):
    '''
        setup_event_monitor function was made to facilitate install and uninstall 
        EventMonitor driver.
        
        PARAMS:
            action= "install" | "uninstall"
        INFO:
            this function uses system call:
                'RUNDLL32.EXE SETUPAPI.DLL,InstallHinfSection <action> 132 <INFO_FILE_PATH>'
    '''
    cmd = "RUNDLL32.EXE SETUPAPI.DLL,InstallHinfSection {ACT} 132 " + INFO_PATH
    act_options = {
        "install": "DefaultInstall",
        "uninstall": "DefaultUninstall"
    }
    action = action.lower()
    
    if action in act_options.keys():
        subprocess.check_call(cmd.format(ACT=act_options[action]), shell=True)
        
        return True
    else:
        print("Unsupported action. Try:")
        print(list(act_options.keys()) )
        
        return False


if __name__ == "__init__":
    event_monitor()
    
