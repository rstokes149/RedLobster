; This Emacs Lisp file defines a C indentation style that closely
; follows most aspects of the one that is used throughout SSLeay,
; and hence in OpenSSL.
; 
; This definition is for the "CC mode" package, which is the default
; mode for editing C source files in Emacs 20, not for the older
; c-mode.el (which is the default in Emacs 19).
;
; Copy the definition in your .emacs file or use M-x eval-buffer.
; To activate this indentation style, visit a C file, type
; M-x c-set-style <RET> (or C-c . for short), and enter "eay".
; To toggle the auto-newline feature of CC mode, type C-c C-a.
;
; Apparently statement blocks that are not introduced by a statement
; such as "if" and that are not the body of a function cannot
; be handled too well by CC mode with this indentation style.
; The style defined below does not indent them at all.
; To insert tabs manually, prefix them with ^Q (the "quoted-insert"
; command of Emacs).  If you know a solution to this problem
; or find other problems with this indentation style definition,
; please send e-mail to bodo@openssl.org.

(c-add-style "eay"
	     '((c-basic-offset . 8)
	       (c-comment-only-line-offset . 0)
	       (c-hanging-braces-alist)
	       (c-offsets-alist	. ((defun-open . +)
				   (defun-block-intro . 0)
				   (block-open . 0)
				   (substatement-open . +)
				   (statement-block-intro . 0)
				   (statement-case-open . +)
				   (statement-case-intro . +)
				   (case-label . -)
				   (label . -)
				   (arglist-cont-nonempty . +)))))