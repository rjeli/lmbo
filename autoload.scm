(use format)
(use srfi-69)
(use lolevel)
(import foreign)
(foreign-declare "#include <GLFW/glfw3.h>")
(foreign-declare "#include \"main.h\"")

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

;; pointer to beginning of buffer
(define fptr 
	(foreign-value "FBBUFFER" (c-pointer char)))

;; put numbers in so we see screen size
;;(move-memory! 
	;"0000000000111111111122222222223333333333444444444455555555566666666667777777777" fptr)

(define (eval-string s)
	(eval (with-input-from-string s read)))

;(define fb-char (pointer-u8-ref FBCHAR))
;(set! FBCHAR #\B)

(define (key k) (char->integer k))

(define *keybindings* (make-hash-table))
(define (bind ch fn)
	(hash-table-set! *keybindings* ch fn))
(define (get-bound-fn k)
	(hash-table-ref/default *keybindings* k #f))

(bind ESC (lambda (win) (set-window-should-close win 1)))

(bind (key #\A) (lambda (w) (pointer-u8-set! fptr (key #\A))))
(bind (key #\B) (lambda (w) (pointer-u8-set! fptr (key #\B))))

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
