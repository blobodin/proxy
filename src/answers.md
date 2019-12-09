Q1:
(1) Reentrant means that the function can be used by multiple threads
at the same time (that is, the function can maintain context across threads).
Successive calls to the function do not break its functionality, regardless of
which threads are calling it and which arguments are being used.

(2) strtok() takes a string as its argument only once, and then NULL is
continually passed in until it parses the appropriate tokens. If multiple
threads attempt to call strtok() on the same string, then strtok() will
produce an error, since only NULL should be passed in while one thread is
parsing the string. In this way, strtok() cannot maintain context between the
different threads. It does not understand why the same string is begin used
as an argument multiple times.
