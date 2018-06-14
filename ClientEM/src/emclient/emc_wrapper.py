from emc import EMClient
import threading
import matplotlib.pyplot as plt
from matplotlib import animation
from numpy import arange


def anim(i, cemw, interval, ax):
    ydata = cemw.current_data
    ax.clear()

    #print("[D] len: {}.".format(len(ydata)) )
    xdata = arange(0.0, len(ydata) * interval, interval)

    # Workaround for not exact calcs
    if len(xdata) == len(ydata):
        ax.plot(xdata, ydata)
    elif len(xdata)+1 == len(ydata):
        ax.plot(xdata[:-1], ydata)



class ThreadPlot(threading.Thread):

    """Docstring for ThreadPlot. """


    def __init__(self, cemw, threadID, name):
        threading.Thread.__init__(self)
        self.cemw = cemw
        self.threadId = threadID
        self.name = name


    def run(self):
        """TODO: Docstring for run.
        :returns: TODO

        """
        print("Starting thread {}".format(self.name))
        try:
            interval = self.cemw.emc.config["interval"]
            fig = plt.figure()
            ax = fig.add_subplot(1, 1, 1)
            an = animation.FuncAnimation( fig, anim, fargs=(self.cemw, interval, ax) , interval=interval*1000)
            plt.ion()
            plt.show()
            while self.cemw.running:
                plt.pause(interval)
            
            print("[V] Done.")
    
            # Plot data
            # Stop!!
            an.event_source.stop()
            an.repeat = False
            plt.ioff()
            plt.show()
        except Exception as e:
            print("Error in thr_plot: " + str(e))


class EMCWrapper():

    """Docstring for EMCWrapper. """

    def __init__(self):
        """TODO: to be defined1. """
        self.emc = EMClient()
        self.exec_data = dict()
        self.exec_count = 0
        self.commands = dict()
        self.__init_commands()
        self.current_data = list()
        self.running = False


    def __init_commands(self):
        """TODO: Docstring for __init_commands.

        :returns: TODO

        """
        self.commands["exit"] = ["q", "quit", "exit"]
        self.commands["start"]  = ["start", "run", "on"]
        self.commands["stop"] = ["stop", "off"]
        self.commands["configure"] = ["configure", "config", "cfg"]
        self.commands["help"] = ["h", "help", "?"]


    def parse_command(self, cmd):
        """TODO: Docstring for parse_command.

        :cmd: TODO
        :returns: TODO

        """
        cmd = cmd.lower()

        if cmd in self.commands["help"]:
            self.help()
        elif cmd in self.commands["configure"]:
            self.configure()
        elif cmd in self.commands["start"]:
            self.run()
        elif cmd in self.commands["stop"]:
            self.stop()
        elif cmd in self.commands["exit"]:
            self.exit()

    
    def help(self):
        """TODO: Docstring for help.
        :returns: TODO

        """
        print("[HELP]")
        print("Commands:")
        for k, v in self.commands.items():
            print(k, end=": ")
            for c in range(len(v)):
                if c < len(v) -1:
                    print(v[c], end=",")
                else:
                    print(v[c])

    def stop(self):
        """TODO: Docstring for stop.
        :returns: TODO

        """
        print("[STOP]")


    def exit(self):
        """TODO: Docstring for function exit.

        :returns: TODO

        """
        print("[EXIT]")


    def configure(self, cfg=None):
        """TODO: Docstring for configure.

        :cfg: TODO
        :returns: TODO

        """
        print("[CONFIGURE]")

        if cfg:
            for k,v in cfg:
                if k in self.emc.config.keys():
                    if type(v) == type(self.emc.config[k]):
                        self.emc.config[k] = v
                    else:
                        print("Invalid type '{}' for 'key {}'.".format(
                            str(type(v)), k))
                else:
                    print("Key '{}' does not exist in the configuration.".format(k))
            return True


        for k, v in self.emc.config.items():
            print("Select new value for '{}' (actual:'{}').".format(k, v) )
            _new = input("> ")
            if _new.replace(' ', ''):
                try:
                    _type = type(v)
                    if _type == str:
                        _new = str(_new)

                    if _type == int:
                        _new = int(_new)

                    if _type == float:
                        _new = float(_new)
                
                    self.emc.config[k] = _new
                except Exception as e:
                    print("Error: " + str(e))
                    return False
            else:
                print("\t(Not change.)")
        return True


    def run(self):
        """TODO: Docstring for run.
        :returns: TODO

        """
        print("[RUN]")

        # Create new data vector for execution
        self.current_data = list()

        try:
            # Start thread of matplotlib
#            th_plot = ThreadPlot(self, self.exec_count, "PlotTest")
#            th_plot.start()

            interval = self.emc.config["interval"]
            fig = plt.figure()
            ax = fig.add_subplot(1, 1, 1)
            an = animation.FuncAnimation( fig, anim, fargs=(self, interval, ax) , interval=interval*1000)
            plt.ion()
            plt.show()

            # Start pebs
            print("Starting execution {:02}.".format(self.exec_count))
            self.running = True
            if not self.emc.start( self.current_data, plot=True):
                print("! Error during emc execution")
            # Increment execution counter

            an.event_source.stop()
            an.repeat = False
            plt.ioff()
            plt.show()

            self.exec_count += 1
            self.exec_data[self.exec_count] = self.current_data
        except Exception as e:
            print("Error in run: " + str(e))

        self.running = False


    def menu(self):
        """TODO: Docstring for menu.
    
        :returns: TODO
    
        """
        cmd = ""
        while cmd not in self.commands["exit"]:
            cmd = input("[EMC]> ")
            self.parse_command(cmd)
        
        print("Exiting")


if __name__ == "__main__":
    #emc = EMClient()
    emcw = EMCWrapper()
    emcw.menu()
    
