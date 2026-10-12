# One chart for the projector slide: speedup vs 1 thread (log scale).
# Inputs are the measured load times (s) per thread count; refresh these from results_summary.md.
import csv, matplotlib; matplotlib.use("Agg")
import matplotlib.pyplot as plt
INK="#1F2933"; RED="#C0392B"; DRED="#7B1E14"; GREEN="#1E8449"; GREY="#8A94A0"
T=[1,2,4,8]
orig=[48.42,203.57,223.98,203.27]     # original parsing (timegm + stod)
cust=[10.70,9.64,8.77,5.51]          # custom date math + from_chars
tg={}
for k,t,s in csv.reader(open("tg_new.csv")): tg.setdefault(k,{})[int(t)]=float(s)
sp_orig=[orig[0]/v for v in orig]; sp_cust=[cust[0]/v for v in cust]
sp_tg=[t*tg["timegm"][1]/tg["timegm"][t] for t in T]    # same work per thread -> throughput vs 1 thread
plt.rcParams.update({"font.family":"DejaVu Sans","axes.spines.top":False,"axes.spines.right":False,"axes.edgecolor":INK,"axes.linewidth":1.5})
fig,ax=plt.subplots(figsize=(8.3,4.55))
x=range(4)
ax.axhline(1,color=GREY,lw=2.5,ls=(0,(4,3)))
ax.plot(x,sp_cust,color=GREEN,lw=6,marker="o",ms=13)
ax.plot(x,sp_orig,color=RED,lw=6,marker="o",ms=13)
ax.plot(x,sp_tg,color=DRED,lw=4,ls=(0,(1.2,1.4)),marker="s",ms=10)
ax.set_xlim(-0.2,3.95); ax.set_ylim(0.065,4.5); ax.set_yscale("log")
ax.set_xticks(list(x)); ax.set_xticklabels(T,fontsize=20,fontweight="bold"); ax.set_xlabel("Threads",fontsize=20,labelpad=4)
ax.set_yticks([0.125,0.25,0.5,1,2,4]); ax.set_yticklabels(["0.1×","0.25×","0.5×","1×","2×","4×"],fontsize=17); ax.minorticks_off()
ax.set_ylabel("Speed vs 1 thread",fontsize=20,labelpad=4)
ax.grid(axis="y",color="#E3E7EC"); ax.set_axisbelow(True)
ax.text(3.0,sp_cust[3]*1.18,"Our fix",color=GREEN,fontsize=22,fontweight="bold",ha="right",va="bottom")
ax.text(3.0,0.40,"Original loader",color=RED,fontsize=22,fontweight="bold",ha="right",va="bottom")
ax.text(3.9,0.072,"timegm call alone",color=DRED,fontsize=19,fontweight="bold",ha="right",va="bottom")
ax.text(3.9,1.08,"faster",color=INK,fontsize=18,fontweight="bold",va="bottom",ha="right")
ax.text(3.9,0.93,"slower",color=INK,fontsize=18,fontweight="bold",va="top",ha="right")
fig.tight_layout(pad=0.5); fig.savefig("big_speedup.png",dpi=250)
import json
json.dump({"orig1":orig[0],"orig8":orig[3],"slow":orig[3]/orig[0],"p1":46.47,"cust8":cust[3],"vsp1":46.47/cust[3],"tg1":tg["timegm"][1],"tg8":tg["timegm"][8],"tgx":tg["timegm"][8]/tg["timegm"][1]},open("stats.json","w"))
print([round(v,2) for v in sp_orig],[round(v,2) for v in sp_cust],[round(v,2) for v in sp_tg])
