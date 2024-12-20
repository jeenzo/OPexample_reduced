# JOBSAP-fatigue

-> [documentation](docs/html/index.html)

<br/>

![[Code diagram.canvas|Code diagram]]

## Run after clone
For numerical experiments comment the line in CMakeLists.txt dedicated to profiling
```
mkdir build; cd build
cmake ..
cmake --build .
./JOBASP-f --help
```

## Generate documentation
Before declaration use a special comment block:
```cpp
/**
 * ... text ...
 */
```
Then, `doxygen dconfig` generates the `docs/html/` folder. `docs/html/index.html` summarizes the documentation

## Generate plots on in/results
Run `in_plots.py` to get the instance wh layout with ordere lines highlighted.
- Specify `python3 in_plots.py --instance instance_folder_name` to generate file based on the selected folder (and save plots there)

Run `out_plots.py` to get plots describing results (for now fatigue evolution for each batch of each picker)
- Specify `python3 out_plots.py --namescheme namescheme_of_results` to read specific results

## General naming scheme
```cpp
int var_name;

void thisIsAFunction();

class ThisIsAClass();

```
