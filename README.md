# Remote Performance Monitor

![png](https://github.com/kotabrog/ft_mini_ls/blob/main/image/ft_mini_ls.gif)

# Overview
Windwos上でLinuxPCのCPU,GPU使用率を確認できます.

Check the CPU and GPU usage of Linux PC remotely from Windows.

# Requirement
- Windows10
- VisualStudio2019
- [OpenSSLurl](https://github.com/openssl/openssl)
- [libssh2url](https://github.com/libssh2/libssh2)

Windows10,Ubuntu20.04LTS において動作確認済み  
Operation has been confirmed on Windows 10 and Ubuntu 20.04LTS.
# Features
- SSHを用いてLinuxPCにコマンドを送信し，情報を得ています．\
Sending commands to a Linux PC using SSH to obtain information.
- 使用しているコマンド(Uage Command)
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

# License
The source code is licensed MIT.

[MIT]https://opensource.org/licenses/mit-license.php