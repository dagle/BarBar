(use-modules (g-golf))
(gi-import "Gtk")

(define (activate app)
  (let ((bar (make <barbar-bar>
                  #:pos 'Bottom))
        (action-bar (make <gtk-action-bar))
        (river-tags (make <barbar-river-tag>))
        (river-view (make <barbar-river-view>))
        (clock (make <barbar-clock>
                  #:format "%k:%M:%S"
                  #:interval 1000))
        (battery (make <barbar-battery>))
        (network (make <barbar-network>
                  #:interface "wlan0"
                  #:interval 1000))
        ; (motion (make <gtk-event-controller-motion>))
        )

    (connect motion 'enter hover battery)
    (connect motion 'leave hover battery)
    (add-controller battery motion)

    (pack-start action-bar river-tags)
    (pack-center action-bar river-view)
    (pack-end action-bar clock)
    (pack-end action-bar battery)
    (pack-end action-bar network)

    (set-child window action-bar)
    (show bar)))

(let ((app (make <gtk-application>
             #:application-id "com.github.barbar")))
  (connect app 'activate activate)
  (run app 0 '()))
