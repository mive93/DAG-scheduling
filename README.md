# DAG-scheduling

make -j && ./demo && dot -Tpng test.dot > test.png 

for i in `seq 0 1 5`; do dot -Tpng test$i.dot > test$i.png; done

To convert from dot to png
```
dot -Tpng test.dot > test.png
```


sudo apt-get install gnuplot 
sudo apt-get install graphviz

sudo apt-get install python3-matplotlib 
sudo apt install libyaml-cpp-dev

