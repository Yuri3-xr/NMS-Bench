# FasterNMS

## Prepare Datasets
```txt
.data
├── coco
│   ├── labels
│   │   ├── 000000000139.csv
│   │   └── ...
│   └── yolo-series
│       ├── v5l-preds
│       │   ├── 000000000139.csv
│       │   └── ...
│       └── ...
└── open-images
    ├── labels
    └── yolo-series
```
## Compilation
Make sure that ``GCC >= 9.0``.
```shell
make clean && make
```

## Runing in Command Line

``run.py`` is used to perform benchmarking of NMS (Non-Maximum Suppression) algorithms. It allows users to specify the paths to prediction files, label files, output directory, and the desired NMS algorithm.

### Usage

Parameters:
1. --preds: Path to the prediction files. Default is ``./data/coco/yolo-series/v8m-preds/``.
2. --labels: Path to the label files. Default is ``./data/coco/labels/``.
3. --output: Path to the output directory. Default is ``./results/``.
4. --method: Name of the NMS algorithm. 

### Examples

1. Running with default parameters:
```python
python run.py
```

2. Running with custom parameters(BOE-NMS as an example):
```python
python run.py --preds ./my_preds/ --labels ./my_labels/ --output ./my_results/ --method BOENMS
```

## Outputs
```txt
BOENMS average latency is 113.608 microseconds
mAP 50:95    = 0.502
mAP 50       = 0.669
mAP 75       = 0.546
```
The average latency is given in microseconds with COCO-style mAP. 