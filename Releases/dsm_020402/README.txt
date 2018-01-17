This is release 2.4.2 of the TWAIN Data Source Manager.

Releases accompany updates to the TWAIN Specification, even when there are no
actual code changes in the DSM.  We do this to reduce confusion even though all
DSMs must interoperate with any application or data source, regardless of their
supported TWAIN Protocol.

Release Notes:

- The Windows DMS sends a NULL for the origin on the DG_CONTROL / DAT_IDENTITY
  / MSG_GET call.  Linux and Mac send the application identity.  This behavior
  can be overridden using the TWAINDSM_USEAPPID environment variable, where a
  value of 1 uses the application id, and 0 sends a NULL.  This fix does not
  change the default behavior for Windows, but addresses crashes on Mac.

