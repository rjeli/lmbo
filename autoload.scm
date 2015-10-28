(use format)
(use srfi-69)
(use lolevel)
(import foreign)
(foreign-declare "#include <GLFW/glfw3.h>")
(foreign-declare "#include \"main.h\"")

(define GLFW_RELEASE 0)
(define GLFW_PRESS 1)
(define GLFW_REPEAT 2)

(define GLFW_MOD_SHIFT 1)
(define GLFW_MOD_CONTROL 2)
(define GLFW_MOD_ALT 4)
(define GLFW_MOD_SUPER 8)

(define fptr (foreign-value "FBBUFFER" (c-pointer char)))
(move-memory! "0000000000111111111122222222223333333333444444444455555555566666666667777777777" fptr)

(define (eval-string s)
	(eval (with-input-from-string s read)))

;(define fb-char (pointer-u8-ref FBCHAR))
;(set! FBCHAR #\B)

(define-foreign-type glfw-window "GLFWwindow")
(define set-window-should-close
	(foreign-lambda void "glfwSetWindowShouldClose" (c-pointer glfw-window) int))

(define *key-table* (make-hash-table))

(define-external (keyCallback ((c-pointer glfw-window) window) (int key) (int scancode) (int action) (int mods)) void
	(if (= 256 key) (set-window-should-close window 1))
	(when (and (>= key 65) (<= key 90) (= GLFW_PRESS action))
		(cond
			((not (= 0 (bitwise-and mods GLFW_MOD_SHIFT))) (format #t "shift ~A" key))
			((not (= 0 (bitwise-and mods GLFW_MOD_CONTROL))) (format #t "ctrl ~A" key))
			(#t (begin (format #t "normal ~A" key)
								 (pointer-u8-set! fptr key))))
		(display "\n")
		;(let ((fn (hash-table-ref/default *key-table* key #f)))
		;	(if fn (fn window)))
		(flush-output)))

(define (bind-key ch fn)
	(hash-table-set! *key-table* ch fn))

(bind-key 256 (lambda (w) (set-window-should-close w 1)))

(display "autoload.scm loaded\n")

(define (key k) (char->integer k))

;(bind-key (key #\A) (lambda (w) (display "this is a test binding!\n")))

(return-to-host)
