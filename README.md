# Blackout

* leveraging gmer driver to effectively disabling or killing EDRs and AVs.
* the sample is sourced from loldrivers https://www.loldrivers.io/drivers/7ce8fb06-46eb-4f4f-90d5-5518a6561f15/
# usage

* Place the driver `Blackout.sys` in the same path as the executable
* The executable should be run in the context of an administrator
* Blackout.exe -p <process_id>
* for windows defender keep the program running to prevent the service from restarting it

  ![image](https://github.com/ZeroMemoryEx/Blackout/assets/60795188/3ea0f7ae-0102-4a38-b4b6-700e93f5d545)
