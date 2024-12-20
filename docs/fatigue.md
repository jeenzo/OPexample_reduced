> At each task execution, the fatigue level of a worker can increase. This leads to degradating performance and can result to the need of dedicated rest time before continuing with picking activities.
# Functions
## Fatigue level
###### A
$$
F_s=F_{s-1}+(1-e^{-p\,\theta})(1-F_{s-1})
$$
## Time increase
###### A
$$
\theta_s=\hat\theta\;(1 + \delta\ln(1+R_{s-1}))
$$

###### B
$$
\theta_s=\hat\theta + (M-\hat\theta)(1-(1+F_{s-1})^{-\delta})
$$

## Recovery
###### A
$$
R_{s-1} = F_{s-1}\,e^{-\mu\tau_{s-1,s}}
$$

## Mandatory rest
Time to reach residual fatigue $R$ starting from a fatigue level of $F$
###### A
$$
\tau(R)=\frac{\ln F/R}{\mu}
$$
following the rest function of [[#Recovery#A]]