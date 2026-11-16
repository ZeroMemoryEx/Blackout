# Blackout

* leveraging gmer driver to effectively disabling or killing EDRs and AVs.
* it bypass HVCI fluently
* the sample is sourced from loldrivers https://www.loldrivers.io/drivers/7ce8fb06-46eb-4f4f-90d5-5518a6561f15/
# usage

* Place the driver `Blackout.sys` in the same path as the executable
* The executable should be run in the context of an administrator
* Blackout.exe -p <process_id>
* for windows defender keep the program running to prevent the service from restarting it

  ![image](https://github.com/ZeroMemoryEx/Blackout/assets/60795188/3ea0f7ae-0102-4a38-b4b6-700e93f5d545)

# NOTE

* Blackout is possibly the first publicly documented modern BYOVD proof of concept demonstrating how a legitimate signed driver can be abused to directly terminate protected AV/EDR processes.

* As mentioned in the write-up, the GMER driver had already been abused by the Conti ransomware group. However, Conti  relied on GMER's graphical user interface to terminate security products, rather than directly interacting with the driver's IOCTL as demonstrated by Blackout, Before this you may find techniques that abuse Process Explorer driver, but they are not as practical because they take time. because they rely on closing process handles and waiting for the kernel to do the job for you.
