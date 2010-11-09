Benutzerschnittstelle:

arfit.h : 		arfit_vector(ar coefficient estimation for one timecourse)
				arfit_matrix(ar coefficient estimation for multiple timecourses)
				arfit_granger(granger causality estimation)
arfit_base.h:	arfit_schneider(schneider algorithm for ar estimation)
				arfit_schloegl(schloegl algorithms for ar estimation)
				arfit_input_alloc/arfit_input_free(macros to allocate/free memory for the input structures used)
				arfit_output_alloc/arfit_output_free(macros to allocate/free memory for the output structures used)
arfit_error.h:	arfit_mse(mean square error)
				arfit_msy(mean square signal)
				arfit_rev(relative error variance)
				arfit_gof(goodnes of fit)
arfit_via.h:	varfit( estimation of univariate ar coefficients for all voxels of a via file containing functional data)
				vgranger( estimation of granger causality values for all voxels of a via file containing functional data)
				get_file_info(get information about functional time data in a given via file)
				add_sample_to_input(add a timecourse sample to a given input structure)
				add_samples_to_input(add multiple timecourse samples to a given input structure)
				detach_sample_from_input(detach a sample from given input structure)
				clear_input(detach all samples from given input structure)


Umsetzung der ARFIT Algorithmen nach Schneider und Schloegl.

Beispielhafte implementierung mit via files siehe vcarfit.c und vgranger.c.

Um die Funktionen arfit_vector, arfit_matrix und arfit_granger nutzen zu können, muss die Datei arfit.h mit aufgenommen werden.
Außerdem entweder die statische Bibliothek arfit.a oder die dynamische Bibliothek arfit.so, dem Linker bekannt gegeben werden.

in arfit_base.h und arfit_via.h befinden sich weitere Funktionen, die das Arbeiten mit via-Files vereinfachen.

Beschreibung der Funktionen:

void arfit_vector( gsl_vector *input, int pmin, int pmax, int zero, arfit_selector selector, arfit_algorithm algorithm, arfit_mode mode, arfit_output **output )

Berechnet die AR-Koeffizienten einer uni-variaten Zeitreihe.

input:	zeigt auf den Vektor, die die Zeitreihendaten beinhaltet.
pmin:	minimale Ordnung des AR-Prozesses.
pmax:	maximale Ordnung des AR-Prozesses.
zero:	0 wenn der Mittelwert der Zeitreihen 0 ist, sonst 1.
output:	zeigt auf einen Pointer, der nach Aufruf der Funktion die Ergebnisse in Form einer Struktur vom Typ arfit_output enthält.


void arfit_matrix( gsl_matrix *input, int pmin, int pmax, int zero, arfit_selector selector, arfit_algorithm algorithm, arfit_mode mode, arfit_output **output )

Berechnet die AR-Koeffizienten einer multi-variaten Zeitreihe. Die Zeitreihen werden dabei in Zeilenordnung angegeben.
Sind 2 Zeitreihen enthalten, so hat die Matrix 2 Zeilen und soviele Spalten, wie die Zeitreihen Observationen.

input:	zeigt auf die Matrix, die die Zeitreihendaten beinhaltet.
pmin:	minimale Ordnung des AR-Prozesses.
pmax:	maximale Ordnung des AR-Prozesses.
zero:	0 wenn der Mittelwert der Zeitreihen 0 ist, sonst 1.
output:	zeigt auf einen Pointer, der nach Aufruf der Funktion die Ergebnisse in Form einer Struktur vom Typ arfit_output enthält.


void arfit_granger( gsl_matrix *input, int pmin, int pmax, int zero, arfit_selector selector, arfit_algorithm algorithm, arfit_mode mode )

Berechnet die Granger-Kausalität bivariater Zeitreihen. Eingabe ist eine Matrix mit 2 Zeilen. In jeder Zeile steht eine
Zeitreihe.
 
input:	zeigt auf die Matrix, die die Zeitreihendaten beinhaltet.
pmin:	minimale Ordnung des AR-Prozesses.
pmax:	maximale Ordnung des AR-Prozesses.
zero:	0 wenn der Mittelwert der Zeitreihen 0 ist, sonst 1.

Diese Funktion gibt ein 3-elementiges Array vom Typ double[3] zurück, welches die 3 Werte der Granger Kausalität beinhaltet.
War die Matrix ungültig, dass heißt nicht 2 zeilig, so liefert die Funktion NULL zurück.

Gemeinsame Parameter:

arfit_selector:		Art des Selektionskriteriums.
					Mögliche Werte:	arfit_selector_sbc (Schwarzs Bayesisches Informationskriterium)
									arfit_selector_fpe (Akaikes Final Prediction Error)

arfit_algorithm:	arfit_algorithm_schneider 	(Schneiders Implementation ARFIT)
					arfit_algorithm_schloegl	(Schloegls Implemenation ARFIT2)

arfit_mode:			Nur relevant falls, arfit_algorithm_schloegl als Parameter gewählt wurde. Schloegls Implementation
					enthält diverse klassische Verfahren zur AR-Koeffizienten-Bestimmung.
					Mögliche Werte:		arfit_mode_notset		(sollte gesetzt werden, wenn arfit_algorithm_schneider gewählt wurde, ist aber optional)
										arfit_mode_ywunbiased	(Yule-Walker mit unbiased Correlation Function)
										arfit_mode_ywbiased		(Yule-Walker mit biased Correlation Function)
										arfit_mode_vmunbiased	(Vieira-Morf mit unbiased Correlation function, default Wert, liefert beste Ergebnisse nach Schloegl)
										arfit_mode_vmbiased		(Vieira-Morf mit biased Correlation function)
										arfit_mode_nsbiased		(Nutall-Strand mit biased Correlation function)
										arfit_mode_nsunbiased	(Nutall-Strand mit unbiased Correlation function)
