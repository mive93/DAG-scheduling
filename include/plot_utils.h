#include "matplotlibcpp.h"

void testMatplotlib(){
    const std::string name = "ciao";
    std::vector<int> x, y;
    x.push_back(1);
    x.push_back(2);
    x.push_back(3);
    x.push_back(4);

    y.push_back(1);
    y.push_back(2);
    y.push_back(3);
    y.push_back(4);

    matplotlibcpp::named_plot(name, x, y);
    matplotlibcpp::annotate("1", 1, 1);
    matplotlibcpp::annotate("2", 2, 2);
    matplotlibcpp::annotate("3", 3, 3);
    matplotlibcpp::annotate("4", 4, 4);
    matplotlibcpp::legend();
    matplotlibcpp::show();
}

void plotResults(std::map<std::string,std::vector<float>> sched_res, std::vector<float> x, const std::string x_axis, const std::string y_axis){
    std::vector<std::string> line_style = {"r", "b", "g--", "k+", "c", "m", "y"};

    int i=0;
    for(std::map<std::string,std::vector<float>>::iterator iter = sched_res.begin(); iter != sched_res.end(); ++iter){
        std::cout<<iter->first<<" "<<iter->second.size()<<" "<<x.size()<<std::endl;
        matplotlibcpp::named_plot(iter->first, x, iter->second, line_style[i++]);
    }

    matplotlibcpp::xlabel(x_axis);
    matplotlibcpp::ylabel(y_axis);
    matplotlibcpp::legend();
    matplotlibcpp::show();

}