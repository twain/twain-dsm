This is release 2.4.3 of the TWAIN Data Source Manager for Windows.  Mac and
Linux remain at 2.4.2.  The only change to Windows was building using Visual
Studio 2017 (previously it was built with 2008).  This was done so that the
code would access newer c-runtime redistributables.

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

