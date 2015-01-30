# read_scheme
program for reading decay information and outputting a GEANT4 friendly format

there is a preprocessor definition in the header that multiplies your input's branching ratios in order to scale down to percentages.
ie.
"#define BR_RAT .01"

so that a branching ratio of 100 or 24 becomes 1.0 , .24 respectively.

