import subprocess
import re

methods = ['GreedyNMS', 'BobNMS', 'AliceNMS', 'UltraAliceNMS']
sizes = ['n', 's', 'm']

results = {}

for method in methods:
    for size in sizes:
        avg_process_time = 0
        avg_map_50_95 = 0
        
        for _ in range(3):
            command = f"python run.py --preds ./data/open-images/yolo-series/v8{size}-preds/ --labels /data02/sl/open-images-v7-gt-csv/ --method {method}"
            output = subprocess.check_output(command, shell=True, universal_newlines=True)
            
            # Extract process time and mAP 50:95
            process_time = re.search(r'process time is (\d+(\.\d+)?) ms', output)
            map_50_95 = re.search(r'mAP 50:95\s+=\s+([\d.]+)', output)
            
            print(output)
            print(float(process_time.group(1)))
            if process_time and map_50_95:
                avg_process_time += float(process_time.group(1))
                avg_map_50_95 += float(map_50_95.group(1))
        
        avg_process_time /= 3
        avg_map_50_95 /= 3
        print(method, size, avg_process_time, avg_map_50_95)
        results[f"{size}-{method}"] = (avg_process_time, avg_map_50_95)

# Output results
for key, value in results.items():
    print(f"{key} - Process Time: {value[0]} ms, mAP 50:95: {value[1]}")
