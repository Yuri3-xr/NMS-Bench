import argparse
import os

def main():
    parser = argparse.ArgumentParser(description='NMS bench parameters')
    parser.add_argument('--preds', default='./data/coco/yolo-series/v8m-preds/', type=str, help='the path of preds')
    parser.add_argument('--labels', default='./data/coco/labels/', type=str, help='the path of labels')
    parser.add_argument('--output', default='./results/', type=str, help='the path of output')
    parser.add_argument('--method', default='BOENMS', type=str, help='the name of NMS algorithm include(OrignalNMS FastNMS FastNMS_Par SoftNMS BOENMS QSINMS eQSINMS ClusterNMS)')

    args = parser.parse_args()
    
    input_pred_file = args.preds
    input_label_file = args.labels
    output_file = args.output
    method = args.method

    command = './build/app/main' + ' ' + input_pred_file + ' ' + input_label_file + ' ' + output_file  + ' ' + method
    os.system(command)

if __name__ == '__main__':
    main()


