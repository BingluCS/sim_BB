import subprocess
from apscheduler.schedulers.blocking import BlockingScheduler
import json


PFS = "/home/ubutnu/hardDisk/PFS/nocompress/"
BB0 = "/home/ubutnu/hardDisk/BB/nocompress/"
BB1 = '/home/ubutnu/hardDisk/BB/nocompress/'

file_json = "/home/ubutnu/Application/Neural-network/Cache/file_cache_nocompress.json"
threshold =   1024*1024*1024*2

def init_bb():
    command = "ls -cr " + BB1 + '* | grep -v \'^' + BB1+ 'bk_\''
    result = subprocess.run(command,shell=True, capture_output=True, text=True)
    file = result.stdout.strip().replace('\n', ' ')
    if len(file) != 0:
        command = "du -csb " + file
        result = subprocess.run(command,shell=True, capture_output=True, text=True)
        output = result.stdout.strip().split()
    else:
        output = ['0','total']
    
    with open(file_json, 'r') as F:
        data = json.load(F)
    file = file.replace(BB1, ' ').split()
    data['ALL']['size'] = float(output[-2])
    data = {key: {'valid': 0 if key in file else 1, 'size': value['size']} for key, value in data.items()}

    with open(file_json, 'w') as F:
        json.dump(data, F, indent=4)
    # with open(BB1, 'r') as F:
    #     data = json.load(F)
    print("running!")
    
    
if __name__ == "__main__":
    # check_interval = 5
    # scheduler = BlockingScheduler()
    # scheduler.add_job(init_bb, 'interval', seconds=check_interval, args=[])
    # scheduler.start()
    init_bb()