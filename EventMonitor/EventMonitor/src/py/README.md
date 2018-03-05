### Config
First configure paths inside `EventMonitor\src\py\project.cfg`

### with IPython:

1. cd to 'EventMonitor\src\py' directory
2. execute: `%run event_monitor.py`

then `em`(EventMonitor) and `c`(Client) are created, use:

```python
# EventMonitor methods:
em.install()
em.uninstall()
em.start()
em.stop()
em.interrogate()

# Client methods:
c.connect()
c.write(text)
c.read()
c.close()


```




