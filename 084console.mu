# Wrappers around interaction primitives that take a potentially fake object
# and are thus easier to test.

exclusive-container event [
  text:character
  keycode:number  # keys on keyboard without a unicode representation
  touch:touch-event  # mouse, track ball, etc.
  resize:resize-event
  # update the assume-console handler if you add more variants
]

container touch-event [
  type:number
  row:number
  column:number
]

container resize-event [
  width:number
  height:number
]

container console [
  index:number
  data:address:array:event
]

recipe new-fake-console events:address:array:event -> result:address:console [
  local-scope
  load-ingredients
  result:address:console <- new console:type
  buf:address:address:array:event <- get-address *result, data:offset
  *buf <- copy events
  idx:address:number <- get-address *result, index:offset
  *idx <- copy 0
]

recipe read-event console:address:console -> result:event, console:address:console, found?:boolean, quit?:boolean [
  local-scope
  load-ingredients
  {
    break-unless console
    idx:address:number <- get-address *console, index:offset
    buf:address:array:event <- get *console, data:offset
    {
      max:number <- length *buf
      done?:boolean <- greater-or-equal *idx, max
      break-unless done?
      dummy:address:event <- new event:type
      reply *dummy, console/same-as-ingredient:0, 1/found, 1/quit
    }
    result <- index *buf, *idx
    *idx <- add *idx, 1
    reply result, console/same-as-ingredient:0, 1/found, 0/quit
  }
  switch  # real event source is infrequent; avoid polling it too much
  result:event, found?:boolean <- check-for-interaction
  reply result, console/same-as-ingredient:0, found?, 0/quit
]

# variant of read-event for just keyboard events. Discards everything that
# isn't unicode, so no arrow keys, page-up/page-down, etc. But you still get
# newlines, tabs, ctrl-d..
recipe read-key console:address:console -> result:character, console:address:console, found?:boolean, quit?:boolean [
  local-scope
  load-ingredients
  x:event, console, found?:boolean, quit?:boolean <- read-event console
  reply-if quit?, 0, console/same-as-ingredient:0, found?, quit?
  reply-unless found?, 0, console/same-as-ingredient:0, found?, quit?
  c:address:character <- maybe-convert x, text:variant
  reply-unless c, 0, console/same-as-ingredient:0, 0/found, 0/quit
  reply *c, console/same-as-ingredient:0, 1/found, 0/quit
]

recipe send-keys-to-channel console:address:console, chan:address:channel, screen:address:screen -> console:address:console, chan:address:channel, screen:address:screen [
  local-scope
  load-ingredients
  {
    c:character, console, found?:boolean, quit?:boolean <- read-key console
    loop-unless found?
    break-if quit?
    assert c, [invalid event, expected text]
    screen <- print-character screen, c
    chan <- write chan, c
    loop
  }
]

recipe wait-for-event console:address:console -> console:address:console [
  local-scope
  load-ingredients
  {
    _, console, found?:boolean <- read-event console
    loop-unless found?
  }
]

# use this helper to skip rendering if there's lots of other events queued up
recipe has-more-events? console:address:console -> result:boolean [
  local-scope
  load-ingredients
  {
    break-unless console
    # fake consoles should be plenty fast; never skip
    reply 0/false
  }
  result <- interactions-left?
]