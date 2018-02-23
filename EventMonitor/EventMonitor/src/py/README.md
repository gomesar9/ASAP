### with IPython:

1. cd to 'src/py' directory
2. execute: `%run event_monitor.py`

`em`(EventMonitor) and `tt`(Test) was created:
use:

```python
# EventMonitor methods:
em.install()
em.uninstall()
em.start()
em.stop()
em.interrogate()

# Test methods:
tt.run('w')	# Write test
tt.run('r') # Read test
tt.run() # Read and write tests

# Pipeline function:
doPipeline(verbose=True, debug=True) # Will install, start, test, and stop driver.

```




