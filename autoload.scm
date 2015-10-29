(use format)
(use srfi-69)
(use lolevel)
(require-extension bind)
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
	float x, y, z;
	float xrot, yrot, zrot;
};
typedef struct panel panel;
panel *add_panel(panel *);
void panel_model_update(panel *);
")
(define-foreign-type raw-panel "panel")
(define-foreign-type panel (c-pointer raw-panel))
(define-foreign-variable root-panel panel "root_panel")

(define (new-panel)
	(let ((p (add-panel root-panel)))
		(set! (panel-x p) 0.0)
		(set! (panel-y p) 0.0)
		(set! (panel-z p) 0.0)
		(set! (panel-xrot p) 0.0)
		(set! (panel-yrot p) 0.0)
		(set! (panel-zrot p) 0.0)
		(panel-model-update p)
		p))

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

(define-foreign-type glfw-window "GLFWwindow")
(define set-window-should-close
	(foreign-lambda void "glfwSetWindowShouldClose" (c-pointer glfw-window) int))

;(define (new-panel)
	;(add_panel root-panel))

;; pointer to beginning of buffer
;(define fptr 
	;(foreign-value "FBBUFFER" (c-pointer char)))

;; put numbers in so we see screen size
;;(move-memory! 
	;"0000000000111111111122222222223333333333444444444455555555566666666667777777777" fptr)

(define (eval-string s)
	(eval (with-input-from-string s read)))

;(define fb-char (pointer-u8-ref FBCHAR))
;(set! FBCHAR #\B)

(define (key k) (char->integer k))

(define *keybindings* (make-hash-table))
(define (bind-key ch fn)
	(hash-table-set! *keybindings* ch fn))
(define (get-bound-fn k)
	(hash-table-ref/default *keybindings* k #f))

; ESC to quit
(bind-key ESC (lambda (win) (set-window-should-close win 1)))

;(bind (key #\A) (lambda (w) (pointer-u8-set! fptr (key #\A))))

(define-external 
	;; called by GLFW on every keypress
	(key_callback ((c-pointer glfw-window) window) 
								(int key-code) 
								(int scancode) 
								(int action) 
								(int mods)) void

	(when (= action GLFW_PRESS)
		(let ((bound-fn (get-bound-fn key-code)))
			(when bound-fn (bound-fn window))))
	)


	;; ESC quits
	;(if (= 256 key-code) (set-window-should-close window 1))
	;(when (and (>= key-code 65) (<= key-code 90) (= GLFW_PRESS action))
		;(cond
			;((not (= 0 (bitwise-and mods GLFW_MOD_SHIFT))) (format #t "shift ~A" key-code))
			;((not (= 0 (bitwise-and mods GLFW_MOD_CONTROL))) (format #t "ctrl ~A" key-code))
			;(#t (begin (format #t "normal ~A" key-code)
								 ;(pointer-u8-set! fptr key-code))))
		;(display "\n")
		;(let ((fn (hash-table-ref/default *key-table* key #f)))
		;	(if fn (fn window)))
		;(flush-output)))

;; (bind-key 256 (lambda (w) (set-window-should-close w 1)))

(display "autoload.scm loaded\n")
(return-to-host)
