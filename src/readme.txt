Ubitrack Faroarm Driver expects FaroArmUsbWrapper.dll/.lib
For compilation use the faroarm SDK provided by faro.

x64 System Compilation:
- you need to compile FaroArmUsbWrapper.dll/.lib yourself, to resolve some common errors with SDK (vERSION 5.6.4.3) do the following:
Copy:
..\FARO\FaroArmUsb SDK\Inc\Win32\FaroAtlServerApp_i.c
-> ..\FARO\FaroArmUsb SDK\Inc\x64\FaroAtlServerApp_i.c

C:\Program Files (x86)\FARO\FaroArmUsb SDK\Inc\Win32\FaroAtlServerApp5.tlb
-> ..\FARO\FaroArmUsb SDK\Inc\x64\FaroAtlServerApp5.tlb

In case of Visual Studion 2010 compilation and link error "LNK1123: failure during conversion to COFF: file invalid or corrupt"
try to install visual Studio 2010 SP 1: 
http://www.microsoft.com/en-us/download/confirmation.aspx?id=23691

(info from http://stackoverflow.com/questions/10888391/error-link-fatal-error-lnk1123-failure-during-conversion-to-coff-file-inval )
