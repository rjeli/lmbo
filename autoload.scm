(use format)
(use srfi-69)
(use lolevel)
(require-extension bind)
(require-extension dyn-vector)
(import foreign)

(foreign-declare "#include <GLFW/glfw3.h>")
(foreign-declare "#include <kazmath/kazmath.h>")
(foreign-declare "#include \"panel.h\"")
(foreign-declare "#include \"main.h\"")

(bind-options
	mutable-fields: #t
	default-renaming: "")
(bind "
___abstract struct panel {
	char *text;
	unsigned int cursor;
	int focused;
	float x, y, z;
	float xrot, yrot, zrot;
};
typedef struct panel panel;
panel *add_panel(panel *);
void panel_model_update(panel *);
void panel_add_char(panel *p, char c);
void panel_add_string(panel *p, char *s);
void panel_clear(panel *p);
")
(define-foreign-type raw-panel "panel")
(define-foreign-type panel (c-pointer raw-panel))
(define-foreign-variable root-panel panel "root_panel")
(define-foreign-variable tracking int "tracking")

(define (TOGGLE-TRACKING)
	(set! tracking 1))

(define *panels* (make-dynvector 0 '()))
(define *current-panel-index* #f)
(define (current-panel)
	(dynvector-ref *panels* *current-panel-index*))

(define (new-panel)
	(let ((p (add-panel root-panel))
				(n-panels (dynvector-length *panels*)))
		(set! (panel-cursor p) 0)
		(set! (panel-x p) 0.0)
		(set! (panel-y p) 0.0)
		(set! (panel-z p) 0.0)
		(set! (panel-xrot p) 0.0)
		(set! (panel-yrot p) 0.0)
		(set! (panel-zrot p) 0.0)
		(panel-model-update p)
		(focus-panel p)
		(dynvector-set! *panels* n-panels p)
		(set! *current-panel-index* n-panels)
		p))

(define (focus-panel p)
	(dynvector-for-each 
		(lambda (i each) (set! (panel-focused each) 0)) *panels*)
	(set! (panel-focused p) 1))

(define (rotate-focus)
	(set! *current-panel-index* (+ *current-panel-index* 1))
	(when (= *current-panel-index* (dynvector-length *panels*))
		(set! *current-panel-index* 0))
	(focus-panel (current-panel)))
(define ROTATE-FOCUS rotate-focus)

(define (move-panel p x y z)
	(set! (panel-x p) x)
	(set! (panel-y p) y)
	(set! (panel-z p) z)
	(panel-model-update p))

;; glfw constants
(define GLFW_RELEASE 0)
(define GLFW_PRESS 1)
(define GLFW_REPEAT 2)

(define GLFW_MOD_SHIFT 1)
(define GLFW_MOD_CONTROL 2)
(define GLFW_MOD_ALT 4)
(define GLFW_MOD_SUPER 8)

(define ESC 256)
(define RET 257)

(define-foreign-type glfw-window "GLFWwindow")
(define set-window-should-close
	(foreign-lambda void "glfwSetWindowShouldClose" (c-pointer glfw-window) int))

(define (eval-string s)
	(eval (with-input-from-string s read)))

(define (key k) (char->integer k))
(define (bounded a b c)
	(and (<= a b) (<= b c)))

(define *keybindings* (make-hash-table))
(define (bind-key ch fn)
	(hash-table-set! *keybindings* ch fn))
(define (get-bound-fn k)
	(hash-table-ref/default *keybindings* k #f))

; ESC to quit
(bind-key ESC 
  (lambda (w s) (set-window-should-close w 1)))

(bind-key (key #\N) 
  (lambda (w s) (new-panel)))

(bind-key (key #\R)
  (lambda (w s) (rotate-focus)))

(bind-key (key #\J)
	(lambda (w s) 
		(if s
			(set! (panel-xrot (current-panel)) (+ (panel-xrot (current-panel)) 0.2))
			(set! (panel-y (current-panel)) (- (panel-y (current-panel)) 0.2)))
		(panel-model-update (current-panel))))
(bind-key (key #\K)
	(lambda (w s) 
		(if s
			(set! (panel-xrot (current-panel)) (- (panel-xrot (current-panel)) 0.2))
			(set! (panel-y (current-panel)) (+ (panel-y (current-panel)) 0.2)))
		(panel-model-update (current-panel))))

(bind-key (key #\H)
	(lambda (w s) 
		(if s
			(set! (panel-yrot (current-panel)) (- (panel-yrot (current-panel)) 0.2))
			(set! (panel-x (current-panel)) (- (panel-x (current-panel)) 0.2)))
		(panel-model-update (current-panel))))
(bind-key (key #\L)
	(lambda (w s) 
		(if s
			(set! (panel-yrot (current-panel)) (+ (panel-yrot (current-panel)) 0.2))
			(set! (panel-x (current-panel)) (+ (panel-x (current-panel)) 0.2)))
		(panel-model-update (current-panel))))

(bind-key (key #\U)
	(lambda (w s) 
		(if s
			(set! (panel-zrot (current-panel)) (- (panel-zrot (current-panel)) 0.2))
			(set! (panel-z (current-panel)) (- (panel-z (current-panel)) 0.2)))
		(panel-model-update (current-panel))))
(bind-key (key #\M)
	(lambda (w s) 
		(if s
			(set! (panel-zrot (current-panel)) (+ (panel-zrot (current-panel)) 0.2))
			(set! (panel-z (current-panel)) (+ (panel-z (current-panel)) 0.2)))
		(panel-model-update (current-panel))))

(bind-key (key #\F)
	(lambda (w s)
		(panel-add-char (current-panel) #\newline)
		(eval-string (panel-text (current-panel)))))

(bind-key RET
	(lambda (w s)
		(panel-add-char (current-panel) #\newline)))

(bind-key (key #\C)
	(lambda (w s)
		(set! (panel-cursor (current-panel)) 0)
		(panel-clear (current-panel))))

(define (IFCONFIG) 
	(panel-add-string (current-panel)
		"lo0: flags=8049<UP,LOOPBACK,RUNNING,MULTICAST> mtu 16384\noptions=3<RXCSUM,TXCSUM>\ninet6 ::1 prefixlen 128\ninet 127.0.0.1 netmask 0xff000000\n"))

(define (UNAME)
	(panel-add-string (current-panel)
		"Darwin Elis-MacBook-Pro.local 14.4.0"))

(define NEW-PANEL new-panel)

(define-external 
	(key_callback ((c-pointer glfw-window) window) 
								(int key-code) 
								(int scancode) 
								(int action) 
								(int mods)) void

	(when (= action GLFW_PRESS)
		(if (and (or (bounded (key #\A) key-code (key #\Z)) 
								 (= key-code (key #\space))
								 (= key-code (key #\-))
								 (= key-code (key #\'))
								 (= key-code (key #\9))
								 (= key-code (key #\0))) 
						 (= 0 (bitwise-and mods GLFW_MOD_CONTROL)))

			(begin
				(cond 
					((= key-code (key #\9)) (set! key-code (key #\( )))
					((= key-code (key #\')) (set! key-code (key #\" )))
					((= key-code (key #\0)) (set! key-code (key #\) ))))
				(panel-add-char (current-panel) (integer->char key-code)))

			(let ((bound-fn (get-bound-fn key-code))
						(shift-pressed (not (= 0 (bitwise-and mods GLFW_MOD_SHIFT)))))
				(when bound-fn (bound-fn window shift-pressed))))))

(display "autoload.scm loaded\n")
(return-to-host)
