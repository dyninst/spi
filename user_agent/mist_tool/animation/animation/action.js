var kCursor = -1;
var kPause = false;

function refresh_canvas(cur) {
  kCursor = cur;
  filename = kCursor;
  filename += ".svg";
  event_id = "Event ";
  event_id += cur;
  $("#event_id").text(event_id);
  event_file = "";
  event_file += kCursor;
  event_file += ".e";
  $("#event").load(event_file);
  $("#canvas").attr("src", filename).dequeue();
}

function init() {
  $(document).ready(
      function() {
        $("#slider" ).slider({
                animate: true,
                range: "min",
                min: 0,
                max: kNumFrames-1,
                step: 1,
                slide: function(event, ui ) {
                },
                change: function(event, ui) {
                  refresh_canvas(ui.value);
                }
          });
      }
   );
}

function play() {
  kCursor = 0;
   for (var i = kCursor; i < kNumFrames; i++) {
    console.log(i)
    $("#canvas").delay(500).queue(
        function(n) {
          console.log(kPause)
           refresh_canvas(kCursor);
          kCursor++;
        });
  }
}

function show_static() {
  $("#canvas").attr("src", "static.svg");
}

function stop() {
  $("#canvas").stop().clearQueue();
}

function next() {
  if (kCursor >= kNumFrames-1) return;
  kCursor++;
  refresh_canvas(kCursor);
}

function previous() {
  if (kCursor <= 0) return;
  kCursor--;
  refresh_canvas(kCursor);
}