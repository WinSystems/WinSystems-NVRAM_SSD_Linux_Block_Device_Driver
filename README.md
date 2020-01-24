# WinSystems-NVRAM_SSD_Linux_Block_Device_Driver

A linux block device driver for the non-volatile SRAM "SSD" on some of WinSystems platforms

1. Install Source Code
  A. Using GIT
      git clone https://github.com/WinSystems/WinSystems-NVRAM_SSD_Linux_Block_Device_Driver.git
      
  B. Using TAR file from WinSystems web site
      tar -xzf winsys_linux_ssd_v1.01.tgz

2. Build Instructions

  cd winsys_ssd
  make clean
  make
  sudo make install
  
3. Loading
  Open the script ssd_load and make sure the IO address in that last line is correct for your platform.
  If it is not correct, edit the file, enter the correct IO address, save and exit.
  
  Enter the following command:
  
  sudo ./ssd_load
  
4. Use fdisk to partition the SSD into a single partition. Format the partition. 

5. Mounting the SSD
  Create a mount point:     mkdir mt_pt
  Mount SSD:                sudo mount /dev/ssd mt_pt


