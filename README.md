# krrp
_krrp_ was written by Jonathan Frech, 2018 to 2019. It is a functional, interpreted and dynamic language which is designed to be cryptic, yet usable. Its Wiki can be found [here](https://github.com/jfrech/krrp/wiki).

# Building
_krrp_ is entirely written in pure C. Apart from recompressing the _krrp_ standard library, only a C compiler is required for building; view the Makefile for specifics.  
`make` should build the entire language, `make stdlib` (requiring Python 3 to be installed) should also recompress the standard library.

# Exemplary prime predicate
As a language appetizer, an implementation of the prime predicate follows.

![Prime predicate.](https://github.com/jfrech/krrp/blob/master/assets/krrp_prime_predicate.png)
