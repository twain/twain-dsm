This is release 2.4.1 of the TWAIN Data Source Manager.

Releases accompany updates to the TWAIN Specification, even when there are no
actual code changes in the DSM.  We do this to reduce confusion even though all
DSMs must interoperate with any application or data source, regardless of their
supported TWAIN Protocol.

Release Notes:

- On macOS and Linux the DSM validates that the .ds being loaded supports the
  machine architecture (ex: don't try to load 32-bit into a 64-bit process).
  If it's not supported, it's ignored.

- There is no change for Windows.


