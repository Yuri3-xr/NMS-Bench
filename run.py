import os

method = str(input())

# input_pred_file = './data/debug/'
input_pred_file = './data/coco_yolov8n/preds/'
input_lable_file = './data/coco_yolov8n/labels/'
# gpu / cpu dataset is a little different, pay attention
output_file = 'result/' + method + '/' 


command = './build/app/main' + ' ' + input_pred_file + ' ' + input_lable_file + ' ' + output_file  + ' ' + method
os.system(command)


print("[test done!]")
