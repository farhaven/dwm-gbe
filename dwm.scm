(use-modules (json))

(define cs-norm (dwm-make-colorscheme "#000" "#ccc"))
(define cs-selected (dwm-make-colorscheme "#fff" "#666"))
(define cs-separator (dwm-make-colorscheme "#666" "#ccc"))
(define cs-separator-selected (dwm-make-colorscheme "#ccc" "#666"))

(define (dwm-notify text)
  (dwm-spawn "notify-send" (string-append " " text " ")))

(dwm-notify "(Re)loaded config")

(define order
  '("date" "net" "apm" "weather" "mpd" "err"))

(define (single-item selected order data maxw flip)
  (let* ((title (car order))
         (item (hash-ref data title))
         (d "")
         (dw (dwm-drw-textw d #t))
         (w (if item
              (dwm-drw-textw item)
              0))
         (dx (- maxw dw w))
         (x (+ dx dw)))
    (when item
      (if (and selected (null? (cdr order)))
        (dwm-drw-set-colorscheme cs-separator)
        (dwm-drw-set-colorscheme cs-separator-selected))
      (dwm-drw-text dx dw d flip #t)
      (if flip
        (dwm-drw-set-colorscheme cs-selected)
        (dwm-drw-set-colorscheme cs-norm))
      (dwm-drw-text x w item))
    (if (null? (cdr order))
      (+ dx (if item 0 dw))
      (single-item selected (cdr order) data (+ dx (if item 0 dw)) (if item (not flip) flip)))))

(dwm-hook-drawstatus
  (lambda (x maxw selected)
    (let* ((s (dwm-status-text))
           (data (catch #t
                    (lambda ()
                      (json-string->scm s))
                    (lambda (key . p)
                      (let ((h (make-hash-table 1)))
                       (hash-set! h "err" "Invalid JSON")
                       h))))
           (rv (single-item selected order data (- maxw (dwm-systray-width)) #f)))
      rv)))
