# ABrANS
We propose an adaptive binary implementation of the range version of Asymmetric Numeral Systems (rANS). 
1. As rANS encoder processes symbols in reverse order, we estimate the probabilities in forward order, store them into the encoder
memory, and use them during the reverse encoding. It guarantees that both the encoder and the decoder have exactly the same
probability estimation for each symbol.
2. We show how this approach can be implemented using probability estimation via Virtual Sliding Window (VSW).
3. We demonstrate that comparing to rANS with a static model, the proposed adaptive binary rANS provides better compression performance having similar decoding complexity.



**References**<br />
[1] E.Belyaev, K. Liu, An adaptive binary rANS with probability estimation in reverse order, IEEE Signal Processing Letters, 2023. <br />
