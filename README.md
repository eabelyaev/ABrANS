# ABrANS
We propose an adaptive binary implementation of the range version of Asymmetric Numeral Systems (rANS). The main difference with the classic rANS are the following:
1. As rANS encoder processes symbols in reverse order, we estimate the probabilities in forward order, store them into the encoder
memory, and use them during the reverse encoding. It guarantees that both the encoder and the decoder have exactly the same
probability estimation for each symbol.
2. We show how this approach can be implemented using probability estimation via Virtual Sliding Window (VSW).

# Performance Evaluation
ABrANS encoder requires two multiplications per input symbol and it needs addtional time to process the memory with the probabilities. Therefore, it is slower than the range coding (ABRC) and classic static rANS. On the other side, ABrANS decoder provides the fastest decoding comparing with the state-of-the-art adaptive binary coders. As a result, ABrANS is 1.5 faster in encoding, and 2.5 faster in decoding than M-Coder, respectively.   

![Speed comparison](./doc/speed.png)

At Large Calgary Corpus the proposed ABrANS provides better compression perforamce comparing with the classic rANS.

![Compression performance comparison](./doc/LargeCalgaryCorpus.png)

One more drawback of the static model in rANS is its low efficiency for relatively short sequences caused by storage of the normalized frequencies into the output bit stream. In order to illustrate it, we performed compression of the first N symbols of The Large Corpus data set. One can see that for N ≤ 16384 the compression performance of rANS is significantly lower due to the frequencies storage, i.e., the proposed adaptive approach is more efficient for short data blocks as well.

![Compression performance comparison](./doc/LargeCorpus.png)


**References**<br />
[1] E.Belyaev, K. Liu, [An adaptive binary rANS with probability estimation in reverse order](https://ieeexplore.ieee.org/document/10283871), IEEE Signal Processing Letters, 2023. <br />
[2] Ian H. Witten, Radford M. Neal, and John G. Cleary, [Arithmetic coding for data compression](https://dl.acm.org/doi/10.1145/214762.214771), Commun. ACM, 1987.
[3] D. Marpe and T. Wiegand, [A highly efficient multiplication-free binary arithmetic coder and its application in video coding](https://ieeexplore.ieee.org/abstract/document/1246667), ICIP, 2003.
[4] E. Belyaev, S. Forchhammer, and K. Liu, [An adaptive multialphabet arithmetic coding based on generalized virtual sliding window](https://ieeexplore.ieee.org/document/7930427), IEEE Signal Processing Letters, 2017.
[5] E. Belyaev, K. Liu, M. Gabbouj, and Y. Li, [An efficient adaptive binary range coder and its VLSI architecture](https://ieeexplore.ieee.org/document/6963444), IEEE Transactions on Circuits and Systems for Video Technology, 2015.
[6] Jarek Duda, [Asymmetric numeral systems: entropy coding combining speed of huffman coding with compression rate of arithmetic coding](https://arxiv.org/abs/1311.2540), 2014.
[7] [Implementation of rANS coder](https://github.com/rygorous/ryg_rans)
