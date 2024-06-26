The prediction by partial matching (PPM) is the state of the art for lossless data compression (in terms of compression efficiency). It uses a finite context length to estimate
the probability of a symbol.
This implementation (PPMC) uses a trie data structure to model the different contexts and generate probabilities. The proababilities
generated are sent to an arithmetic encoder to encode them into bits.

Compression Efficiency:
I tested the algorithm on several files with very good compression ratios, usually (4-5) : 1, and in one case i got a compression ratio of about
100 (the file content was very skewed).
The compression efficiency was slightly improved after implenting exclusion.

Memory Usage:
The memory requirement for the PPM is very large. However with the use of a list instead os a vector in the trie, the memory consumption was
reduced by an order of ~50

Compression speed:
The compression speed of this implementation averages at about 1.7 mb/second.

system specs: core i5, 8GB RAM, 1.9GHZ processor speed.

The table below shows the compression efficiency of this implementation when compressing the calgary corpus. I also include the
results of Cleary and Witten implementation for comparisons.

| file    | size(bytes) | PPMC(Witten) | PPMC(mine)(order-4) |
| ------- | ----------- | ------------ | ------------------- |
| bib     | 111261      | 2.11         | 1.99                |
| book1   | 768771      | 2.48         | 2.33                |
| book2   | 610856      | 2.26         | 2.06                |
| geo     | 102400      | 4.78         | 4.92                |
| news    | 377109      | 2.65         | 2.48                |
| obj1    | 21504       | 3.76         | 3.91                |
| obj2    | 246814      | 2.69         | 2.61                |
| paper1  | 53161       | 2.48         | 2.40                |
| paper2  | 82199       | 2.45         | 2.36                |
| pic     | 513216      | 1.09         | 1.27                |
| progc   | 39611       | 2.49         | 2.47                |
| progl   | 71646       | 1.90         | 1.84                |
| progp   | 49379       | 1.84         | 1.83                |
| trans   | 93695       | 1.77         | 1.65                |
| average | 224402      | 2.48         | 2.43                |
