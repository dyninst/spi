var kCursor = -1;

function refresh_canvas(cur) {
  kCursor = cur
  filename = kCursor;
  filename += ".svg";
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
  for (var i = 0; i < kNumFrames; i++) {
    $("#canvas").delay(1000).queue(
        function(n) {
          refresh_canvas(kCursor);
          kCursor++;
        });
  }
}

function show_static() {
  $("#canvas").attr("src", "static.svg");
}