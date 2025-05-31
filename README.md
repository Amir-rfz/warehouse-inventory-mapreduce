# warehouse-inventory-mapreduce

## Overview

A C++ utility that forks one process per product, streams each warehouse’s transaction CSVs through Linux pipes in a MapReduce-like pattern, and outputs a consolidated inventory and profit report.

## Input Files

1. **`parts.csv`**
   
   - Header: `product,quantity,price_per_unit,warehouse_name`
     
   - Each row defines opening stock and cost for one product in one warehouse.

2. **`<WarehouseName>.csv`**
   
   - Header: `product,quantity,price_per_unit,type`
     
   - Each row represents either restock (`type=input`) or sale (`type=output`).
     
   - If a sale exceeds available stock, the program prints an error and exits.

## Build & Run

1. **Build**

   ```bash
   cd warehouse-inventory-mapreduce
   make
   ```
  This will:
  
  * Compile all `.cpp` files located in `src/`
    
  * Place intermediate `.o` files into the `build/` directory
    
  * Produce the following executables in the root:
  
    * `warehouseManager.out` – main program (parent/Reduce logic)
      
    * `store.out` – store (warehouse) logic
      
    * `part.out` – product (child/Map) logic



3. **Run**

  ```bash
  ./warehouseManager.out data/parts.csv data/
  ```

   * **Arg 1**: Path to `parts.csv`.
   * **Arg 2**: Path to the `data/` folder containing all warehouse CSVs.

The parent will spawn one mapper per product, send each transaction line down `/tmp/<product>.fifo`, and then collect per-product summaries to print:

```
================ Inventory & Profit Report ================

Product: <name>
Warehouse  | Remaining Qty | Profit (IRR)
… (rows per warehouse) …
Total Profit for <product>: <value> IRR

… (repeat per product) …

Grand Total Profit (All Products, All Warehouses): <value> IRR
==============================================================
```

