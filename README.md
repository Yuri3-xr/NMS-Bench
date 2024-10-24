# NMS-Bench

NMS-Bench is a robust framework that allows researchers to evaluate various NMS methods over
different models and datasets in a few minutes. NMS-Bench primarily consists of three components:
original bounding box data without NMS applied, implementations of various NMS algorithms as
benchmarking methods, and evaluation metrics.

## Prepare Datasets

File structure of NMS-Bench:

```txt
.
├── LICENSE
├── Makefile
├── README.md
├── run.py
├── src
│   └── ...
└── data
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

You can download data from [here](https://drive.google.com/file/d/1oSRNNWTv2QP3PwKkZH9z-cxcLMXdlmL4/view?usp=sharing). You can also generate the data yourself, but please ensure that the filenames of the model output match the filenames of the labels.

## Requirements

- GCC >= 9
- Python 3.X.X

## Compilation

Run:

```shell
make clean && make
```

In some methods, we use execution policies (std::execution::par, std::execution::unseq, std::execution::par_unseq) to support parallelism, such as Fast NMS and Cluster-NMS. On some platforms (like macOS with Clang and libc++), these execution policies may not be fully implemented or supported without additional setup. 

We recommend using GCC and executing on x86_64.

## Runing in Command Line

``run.py`` is used to perform benchmarking of NMS (Non-Maximum Suppression) algorithms. It allows users to specify the paths to prediction files, label files, output directory, and the desired NMS algorithm.

### Usage

Parameters:
1. --preds: Path to the prediction files. By default, it is ``./data/coco/yolo-series/v8m-preds/``.
2. --labels: Path to the label files. By default, it is ``./data/coco/labels/``.
3. --output: Path to the output directory. By default, it is ``./results/``.
4. --method: Name of the NMS algorithm. By default, it is ``BOENMS``.

### Examples

1. Running with default parameters:
```python
python run.py
```

2. Running with custom parameters (Taking BOE-NMS as an example):
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

## Citation

```
@article{si2024accelerating,
  title   = {Accelerating Non-Maximum Suppression: A Graph Theory Perspective},
  author  = {Si, King-Siong and Sun, Lu and Zhang, Weizhan and Gong, Tieliang and Wang, Jiahao and Liu, Jiang and Sun, Hao},
  journal = {arXiv preprint arXiv:2409.20520},
  year    = {2024}
}
```
