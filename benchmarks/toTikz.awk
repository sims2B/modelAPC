BEGIN{
    tikz = 0;
    print "\\documentclass{standalone}\n\\usepackage{pgfplots}\n\\usepackage{tikz}\n\\begin{document}"
}{
    if( $0 ~ /begin{tikzpicture}/) {tikz = 1}
    if(tikz) {print $0}
    if( $0 ~  /end{tikzpicture}/ ) {tikz = 0}
}
 END {
     print "\\end{document}"
 
 }
