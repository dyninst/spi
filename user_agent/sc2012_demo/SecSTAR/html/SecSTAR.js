//-------------------------------------------------------------
// Main data structures
//-------------------------------------------------------------

// This class contains all information needed for visualization
function VizPkg(is_dedup,
                dir) {
    this.is_dedup = is_dedup; // VizPkg has only two instances
    // - is_dedup = true or false

    this.dir = dir;           // Directory path that holds everything

    // For animation
    // Note: all arrays are 1-based index
    this.svgs = null;         // Array of contents of svgs for animation
    this.events = null;       // Array of contents of event description
    this.num_frames = 0;      // Number of frames in animation
    this.cursor = -1;         // Current frame

    this.delimiter_svgs = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>";
    this.delimiter_events = "===event===";

    // For static display
    this.static_svg = "";     // Content of static svg (in xml)

    this.inc_cursor = function() {
        if (this.cursor < this.num_frames-1) {
            this.cursor++;
        }
    }

    this.dec_cursor = function() {
        if (this.cursor > 0) {
            this.cursor--;
        }
    }

    this.dump = function() {

        console.log("===== SVGS ======");
        for (var i = 1; i <= this.num_frames; i++) {
            console.log(this.svgs[i])
        }

        console.log("===== Events ======");
        for (var i = 1; i <= this.num_frames; i++) {
            console.log(this.events[i])
        }
    }

}

//-------------------------------------------------------------
// Global variables
//-------------------------------------------------------------

// Vizualization objects
var viz = new VizPkg(false, "images/raw");
var viz_dedup = new VizPkg(true, "images/dedup");
var cur_viz = viz;

// To assist data loading. When it is 0, then we finish loading
var kDone = 4;

// Some configuration parameters
var kZoom = 100;
var kZoomStep = 10;
var kZoomMax = 400;
var kZoomMin = 10;

var kMinDelay = 50;
var kMaxDelay = 1000;
var kDelay = 200;

//-------------------------------------------------------------
// Public functions for different events in html
//-------------------------------------------------------------

//
// Update speed meter
//
function update_speed() {
		var str = "Speed: ";
		str += Math.round((1-kDelay / kMaxDelay) * 100);
		str += "%";
    $("#speed_meter").html(str);
}

//
// Update event labels
//
function update_event(is_static) {
    total = cur_viz.num_frames;
    $("#event_title").text('Events (total:' + total + ')');

    if (is_static) {
        $("#event_id").text(' ');
        $("#event").html("Static Figure");
    }
}

//
// Update frame diagram and event description
//
function refresh_canvas(cursor) {
    cur_viz.cursor = cursor;

    var event_id = "Event ";
    event_id += (cursor+1);
    $("#event_id").text(event_id);
    $("#event").html(cur_viz.events[cursor+1]);
    $("#canvas").html(cur_viz.svgs[cursor+1]).dequeue();
    $("#slider" ).slider({
        value: cursor+1,
    }  );
}

//
// Onclick event handler for first button
//
function first() {
    refresh_canvas(0);
}

//
// Onclick event handler for last button
//
function last() {
    refresh_canvas(cur_viz.num_frames-1);
}

//
// Onclick event handler for next button
//
function next() {
    cur_viz.inc_cursor();
    refresh_canvas(cur_viz.cursor);
}

//
// Onclick event handler for next button
//
function previous() {
    cur_viz.dec_cursor();
    refresh_canvas(cur_viz.cursor);
}

//
// Onclick event handler for play button
// When this button is pressed, other buttons are disabled
//
function play() {
    if (cur_viz.cursor == cur_viz.num_frames-1) cur_viz.cursor = -1;
    var init_i = cur_viz.cursor;

    for (var i = init_i; i <= cur_viz.num_frames; i++) {
        $("#canvas").delay(kDelay).queue(
            function() {
                next();
            }
        );
    }

    $('#play_btn').attr('disabled','disabled');
    $('#first_btn').attr('disabled','disabled');
    $('#previous_btn').attr('disabled','disabled');
    $('#next_btn').attr('disabled','disabled');
    $('#last_btn').attr('disabled','disabled');
    $('#static_btn').attr('disabled','disabled');
    $('#dedup').attr('disabled','disabled');
    $('#faster_btn').attr('disabled','disabled');
    $('#slower_btn').attr('disabled','disabled');
    $('#stop_btn').removeAttr('disabled');
    $("#slider" ).slider({disabled:true});

    $("#canvas").delay(kDelay).queue(
        function() {
            $('#play_btn').removeAttr('disabled');
            $('#first_btn').removeAttr('disabled');
            $('#previous_btn').removeAttr('disabled');
            $('#next_btn').removeAttr('disabled');
            $('#last_btn').removeAttr('disabled');
            $('#static_btn').removeAttr('disabled');
            $('#dedup').removeAttr('disabled');
            $('#stop_btn').attr('disabled', 'disabled');
            $("#slider" ).slider({disabled:false});
						if (kDelay >= kMinDelay) {
								$('#faster_btn').removeAttr('disabled');
						}
						if (kDelay < kMaxDelay) {
								$('#slower_btn').removeAttr('disabled');
						}
        }
    );
}

//
// Onclick event handler for stop button
//
function stop() {
    $("#canvas").stop().clearQueue();

    $('#play_btn').removeAttr('disabled');
    $('#first_btn').removeAttr('disabled');
    $('#previous_btn').removeAttr('disabled');
    $('#next_btn').removeAttr('disabled');
    $('#last_btn').removeAttr('disabled');
    $('#static_btn').removeAttr('disabled');
    $('#dedup').removeAttr('disabled');
    $('#stop_btn').attr('disabled', 'disabled');
    $("#slider" ).slider({disabled:false});
		if (kDelay >= kMinDelay) {
				$('#faster_btn').removeAttr('disabled');
		}
		if (kDelay < kMaxDelay) {
				$('#slower_btn').removeAttr('disabled');
		}
}

//
// Onclick event handler for zoomin btn
//
function zoomin() {
    if (kZoom >= kZoomMax) return;

    kZoom += kZoomStep;

    if (jQuery.browser.mozilla) {
        // For firefox
        var moz_zoomstr = "scale(";
        moz_zoomstr += (kZoom / 100);
        moz_zoomstr += ")";
        document.getElementById('canvas').style.MozTransform = moz_zoomstr;
        document.getElementById('canvas').style.MozTransformOrigin = "0 0";
    } else {
        // For chrome
        var zoomstr = kZoom;
        zoomstr += '%';
        canvas.style.zoom = zoomstr;
    }
}


//
// Onclick event handler for zoomout btn
//
function zoomout() {
    if (kZoom <= kZoomMin) return;

    kZoom -= kZoomStep;

    if (jQuery.browser.mozilla) {
        // For firefox
        var moz_zoomstr = "scale(";
        moz_zoomstr += (kZoom / 100);
        moz_zoomstr += ")";
        document.getElementById('canvas').style.MozTransform = moz_zoomstr;
        document.getElementById('canvas').style.MozTransformOrigin = "0 0";
    } else {
        // For chrome
        var zoomstr = kZoom;
        zoomstr += '%';
        canvas.style.zoom = zoomstr;
    }
}

//
// Onclick event handler for groupping identical nodes
//
function dedup() {
    if ($("#dedup").is(':checked') == true) {
        cur_viz = viz_dedup;
    } else {
        cur_viz = viz;
    }
    update_event(true);
    first();
    show_static();
    cur_viz.cursor = -1;
}

//
// Update slider
//
function update_slider() {
    var num_frames = cur_viz.num_frames;

    $("#slider" ).slider({
        animate: true,
        range: "min",
        min: 0,
        max: num_frames-1,
        step: 1,
        slide: function(event, ui ) {
        },
        slide: function(event, ui) {
            refresh_canvas(ui.value);
        }
    });
}

//
// Display static diagram
//
function show_static() {
    if (cur_viz.static_svg != "") {
        console.log("static svg from cache");
        $("#canvas").html(cur_viz.static_svg);
        update_event(true);
        update_slider();
        return;
    }

    var file_path = cur_viz.dir;
    var zipFs = new zip.fs.FS();
    function onerror(message) {
        console.error(message);
    }
    function load_data(text) {
        cur_viz.static_svg = text;
        $("#canvas").html(text);
        update_event(true);
        update_slider();
    }
    zip.workerScriptsPath = "lib/";
    file_path += "/static.zip";
    console.log(file_path)
    zipFs.importHttpContent(file_path, false, function() {
        var firstEntry = zipFs.root.children[0];
        firstEntry.getText(function(data) {
            load_data(data);
        });
    }, onerror);
}

//
// This will be invoked for onload event of <body>
//
function init() {

    function show_legend() {
        // Show abbr_exe legend
        abbr_file = viz.dir;
        abbr_file += "/legend/abbr_exe.txt";
        $("#abbr_exe").load(abbr_file);

        // Show users
        user_file = viz.dir;
        user_file += "/legend/color_user.svg";
        $("#color_user").load(user_file);

        // Show nodes and edges
        shape_file = viz.dir;
        shape_file += "/legend/shape.svg";
        $("#shape").load(shape_file);
    }

    //
    // Show UI and hide waiting icon
    //
    function show_ui() {
        console.log("Show UI");
        $('#stop_btn').attr('disabled','disabled');
        $("#framecontentLeft").show();
        $("#maincontent").show();
        $("#framecontentRight").show();
        $("#framecontentBottom").show();
        $("#progress").hide();
        show_legend();
        show_static();
				update_speed();
    }

    //
    // Load zip file and unzip it
    // We use delimiter to determine whether we are dealing with svgs or events
    //
    function get_data(delimiter,
                      output) {
        var zipFs = new zip.fs.FS();

        function onerror(message) {
            console.error(message);
        }

        function load_data(text) {
            if (delimiter == output.delimiter_svgs) {
                output.svgs = text.split(delimiter);
                output.num_frames = output.svgs.length - 1;
                console.log(output.num_frames);
            } else {
                output.events = text.split(delimiter);
                output.num_frames = output.events.length - 1;
                console.log(output.num_frames);
            }

            // See if we can show ui!
            kDone--;
            if (kDone == 0) {
                show_ui();
            }
        }

        zip.workerScriptsPath = "lib/";
        file_path = output.dir;
        if (delimiter == output.delimiter_svgs) {
            file_path += "/svgs.zip";
        } else {
            file_path += "/events.zip";
        }

        console.log(file_path)
        zipFs.importHttpContent(file_path, false, function() {
            var firstEntry = zipFs.root.children[0];
            firstEntry.getText(function(data) {
                load_data(data);
            });
        }, onerror);
    }

    // The starting point of the entire initialization process
    $(document).ready(
        function() {
            get_data(viz.delimiter_svgs, viz);
            get_data(viz.delimiter_events, viz);
            get_data(viz.delimiter_svgs, viz_dedup);
            get_data(viz.delimiter_events, viz_dedup);
        }
    );
}

function faster() {
		if (kDelay >= kMinDelay) {
				kDelay -= kMinDelay;
				update_speed();
				$('#slower_btn').removeAttr('disabled');
		} else {
				$('#faster_btn').attr('disabled','disabled');
		}
}

function slower() {
		if (kDelay < kMaxDelay) {
				kDelay += kMinDelay;
				update_speed();
				$('#faster_btn').removeAttr('disabled');
		} else {
				$('#slower_btn').attr('disabled','disabled');
		}
}