# Remote Performance Monitor

![png](https://github.com/nakai-kosuke/remote_performance_monitor/blob/main/img/Show.png)

# Overview
Windwos上でLinuxPCのCPU,GPU使用率を確認できます.

Check the CPU and GPU usage of Linux PC remotely from Windows.

# Requirement
- Windows10
- [OpenSSL](https://github.com/openssl/openssl)
- [libssh2](https://github.com/libssh2/libssh2)

Windows10,Ubuntu20.04LTS において動作確認済み  
Operation has been confirmed on Windows 10 and Ubuntu 20.04LTS.
# Features
- SSHを用いてLinuxPCにコマンドを送信し，情報を得ています．\
Sending commands to a Linux PC using SSH to obtain information.   
- 前回の接続情報はAES-128を用いて暗号化して保存し，次回起動時に自動的に接続を行います．  
The previous connection information is encrypted and saved using AES-128, and the connection is automatically established at the next startup.
- 使用しているコマンド(Use Command)
    - CPU Info
        - vmstat
        - free
        - date +%s
        - cat /sys/devices/platform/coretemp.?/hwmon/hwmon?/temp?_input 
        - cat /proc/cpuinfo | grep "model name" | uniq
        - cat /sys/devices/system/cpu/cpu?/cpufreq/scaling_cur_freq
        - cat /proc/uptime
        - ps auc  
    - GPU Info
        - nvidia-smi --format=csv,noheader,nounits --query-gpu=index,
                            uuid,name,timestamp,memory.total,memory.used,\
                            utilization.gpu,utilization.memory,temperature.gpu,power.draw,power.limit
        - nvidia-smi --format=csv,noheader,nounits --query-compute-apps=gpu_uuid,pid,process_name,used_memory

# Build
- Requirement
    - VisualStudio2019
    - CMake GUI
    - WindowsSDK
    - [OpenSSL](https://github.com/openssl/openssl)
    - [libssh2](https://github.com/libssh2/libssh2)

- Install  
Cmake GUIを使用してプロジェクトを生成し，VisualStudioでBuildしてください.  
Generate the project using Cmake GUI, and Build it with VisualStudio.

# License
The source code is licensed MIT.

[MIT]https://opensource.org/licenses/mit-license.php
