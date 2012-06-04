var labelType, useGradients, nativeTextSupport, animate;

(function() {
  var ua = navigator.userAgent,
      iStuff = ua.match(/iPhone/i) || ua.match(/iPad/i),
      typeOfCanvas = typeof HTMLCanvasElement,
      nativeCanvasSupport = (typeOfCanvas == 'object' || typeOfCanvas == 'function'),
      textSupport = nativeCanvasSupport 
        && (typeof document.createElement('canvas').getContext('2d').fillText == 'function');
  //I'm setting this based on the fact that ExCanvas provides text support for IE
  //and that as of today iPhone/iPad current text support is lame
  labelType = (!nativeCanvasSupport || (textSupport && !iStuff))? 'Native' : 'HTML';
  nativeTextSupport = labelType == 'Native';
  useGradients = nativeCanvasSupport;
  animate = !(iStuff || !nativeCanvasSupport);
})();

var Log = {
  elem: false,
  write: function(text){
    if (!this.elem) 
      this.elem = document.getElementById('log');
    this.elem.innerHTML = text;
    this.elem.style.left = (500 - this.elem.offsetWidth / 2) + 'px';
  }
};


function init(){
  // init data
var json = [
{"adjacencies": [
], 
"data": {
"$color": "#BDBDBD",
"$type": "circle"
},
"id": "pid=12267@192.168.8.133",
"name": "/bin/sh"
},
{"adjacencies": [
{
"nodeFrom": "pid=12293@192.168.8.133",
"nodeTo": "pid=12267@192.168.8.133",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "circle"
},
"id": "pid=12293@192.168.8.133",
"name": "/usr/sbin/condor_master"
},
{"adjacencies": [
{
"nodeTo": "pid=4610@192.168.8.135",
"nodeFrom": "pid=12317@192.168.8.133",
"data": {}
},
{
"nodeTo": "pid=12299@192.168.8.133",
"nodeFrom": "pid=12317@192.168.8.133",
"data": {}
},
{
"nodeTo": "pid=6733@192.168.8.134",
"nodeFrom": "pid=12317@192.168.8.133",
"data": {}
},
{
"nodeTo": "pid=12298@192.168.8.133",
"nodeFrom": "pid=12317@192.168.8.133",
"data": {}
},
{
"nodeFrom": "pid=12317@192.168.8.133",
"nodeTo": "pid=12298@192.168.8.133",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "circle"
},
"id": "pid=12317@192.168.8.133",
"name": "condor_negotiator"
},
{"adjacencies": [
{
"nodeTo": "pid=12298@192.168.8.133",
"nodeFrom": "pid=12299@192.168.8.133",
"data": {}
},
{
"nodeTo": "pid=12299@192.168.8.133",
"nodeFrom": "pid=12299@192.168.8.133",
"data": {}
},
{
"nodeFrom": "pid=12299@192.168.8.133",
"nodeTo": "pid=12298@192.168.8.133",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "circle"
},
"id": "pid=12299@192.168.8.133",
"name": "condor_collector"
},
{"adjacencies": [
{
"nodeTo": "pid=12299@192.168.8.133",
"nodeFrom": "pid=12298@192.168.8.133",
"data": {}
},
{
"nodeFrom": "pid=12298@192.168.8.133",
"nodeTo": "pid=12293@192.168.8.133",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FEE6CE",
"$type": "circle"
},
"id": "pid=12298@192.168.8.133",
"name": "/usr/sbin/condor_master"
},
{"adjacencies": [
{
"nodeFrom": "pid=4642@192.168.8.135",
"nodeTo": "pid=4609@192.168.8.135",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "triangle"
},
"id": "pid=4642@192.168.8.135",
"name": "condor_shadow"
},
{"adjacencies": [
{
"nodeFrom": "pid=4625@192.168.8.135",
"nodeTo": "pid=4610@192.168.8.135",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "triangle"
},
"id": "pid=4625@192.168.8.135",
"name": "condor_procd"
},
{"adjacencies": [
{
"nodeTo": "pid=4610@192.168.8.135",
"nodeFrom": "pid=4635@192.168.8.135",
"data": {}
},
{
"nodeFrom": "pid=4635@192.168.8.135",
"nodeTo": "pid=4634@192.168.8.135",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#31A354",
"$type": "triangle"
},
"id": "pid=4635@192.168.8.135",
"name": "condor_submit"
},
{"adjacencies": [
{
"nodeTo": "pid=12317@192.168.8.133",
"nodeFrom": "pid=4610@192.168.8.135",
"data": {}
},
{
"nodeTo": "pid=4609@192.168.8.135",
"nodeFrom": "pid=4610@192.168.8.135",
"data": {}
},
{
"nodeTo": "pid=6733@192.168.8.134",
"nodeFrom": "pid=4610@192.168.8.135",
"data": {}
},
{
"nodeTo": "pid=12299@192.168.8.133",
"nodeFrom": "pid=4610@192.168.8.135",
"data": {}
},
{
"nodeFrom": "pid=4610@192.168.8.135",
"nodeTo": "pid=4609@192.168.8.135",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "triangle"
},
"id": "pid=4610@192.168.8.135",
"name": "condor_schedd"
},
{"adjacencies": [
{
"nodeTo": "pid=12299@192.168.8.133",
"nodeFrom": "pid=4609@192.168.8.135",
"data": {}
},
{
"nodeFrom": "pid=4609@192.168.8.135",
"nodeTo": "pid=4604@192.168.8.135",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FEE6CE",
"$type": "triangle"
},
"id": "pid=4609@192.168.8.135",
"name": "/usr/sbin/condor_master"
},
{"adjacencies": [
], 
"data": {
"$color": "#BDBDBD",
"$type": "triangle"
},
"id": "pid=4571@192.168.8.135",
"name": "/bin/sh"
},
{"adjacencies": [
{
"nodeFrom": "pid=4626@192.168.8.135",
"nodeTo": "pid=4610@192.168.8.135",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FEE6CE",
"$type": "triangle"
},
"id": "pid=4626@192.168.8.135",
"name": "/usr/sbin/condor_shadow"
},
{"adjacencies": [
{
"nodeFrom": "pid=4604@192.168.8.135",
"nodeTo": "pid=4571@192.168.8.135",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "triangle"
},
"id": "pid=4604@192.168.8.135",
"name": "/usr/sbin/condor_master"
},
{"adjacencies": [
{
"nodeFrom": "pid=4611@192.168.8.135",
"nodeTo": "pid=4609@192.168.8.135",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "triangle"
},
"id": "pid=4611@192.168.8.135",
"name": "condor_shadow"
},
{"adjacencies": [
{
"nodeFrom": "pid=4627@192.168.8.135",
"nodeTo": "pid=4609@192.168.8.135",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "triangle"
},
"id": "pid=4627@192.168.8.135",
"name": "condor_shadow"
},
{"adjacencies": [
{
"nodeFrom": "pid=4623@192.168.8.135",
"nodeTo": "pid=4611@192.168.8.135",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FEE6CE",
"$type": "triangle"
},
"id": "pid=4623@192.168.8.135",
"name": "condor_shadow"
},
{"adjacencies": [
{
"nodeTo": "pid=4610@192.168.8.135",
"nodeFrom": "pid=4641@192.168.8.135",
"data": {}
},
{
"nodeTo": "pid=6733@192.168.8.134",
"nodeFrom": "pid=4641@192.168.8.135",
"data": {}
},
{
"nodeFrom": "pid=4641@192.168.8.135",
"nodeTo": "pid=4610@192.168.8.135",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "triangle"
},
"id": "pid=4641@192.168.8.135",
"name": "condor_shadow"
},
{"adjacencies": [
], 
"data": {
"$color": "#BDBDBD",
"$type": "triangle"
},
"id": "pid=4634@192.168.8.135",
"name": "sh"
},
{"adjacencies": [
{
"nodeFrom": "pid=6734@192.168.8.134",
"nodeTo": "pid=6732@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "star"
},
"id": "pid=6734@192.168.8.134",
"name": "condor_starter"
},
{"adjacencies": [
{
"nodeFrom": "pid=6775@192.168.8.134",
"nodeTo": "pid=6732@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "star"
},
"id": "pid=6775@192.168.8.134",
"name": "condor_starter"
},
{"adjacencies": [
{
"nodeFrom": "pid=6750@192.168.8.134",
"nodeTo": "pid=6733@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FEE6CE",
"$type": "star"
},
"id": "pid=6750@192.168.8.134",
"name": "/usr/lib/condor/libexec/condor_ckpt_probe"
},
{"adjacencies": [
], 
"data": {
"$color": "#BDBDBD",
"$type": "star"
},
"id": "pid=1@192.168.8.134",
"name": "init"
},
{"adjacencies": [
{
"nodeFrom": "pid=6777@192.168.8.134",
"nodeTo": "pid=6733@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FEE6CE",
"$type": "star"
},
"id": "pid=6777@192.168.8.134",
"name": "mips"
},
{"adjacencies": [
{
"nodeFrom": "pid=6761@192.168.8.134",
"nodeTo": "pid=6733@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FEE6CE",
"$type": "star"
},
"id": "pid=6761@192.168.8.134",
"name": "/usr/lib/condor/libexec/condor_power_state"
},
{"adjacencies": [
{
"nodeFrom": "pid=6748@192.168.8.134",
"nodeTo": "pid=6734@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FEE6CE",
"$type": "star"
},
"id": "pid=6748@192.168.8.134",
"name": "condor_starter"
},
{"adjacencies": [
], 
"data": {
"$color": "#BDBDBD",
"$type": "star"
},
"id": "pid=6689@192.168.8.134",
"name": "/bin/sh"
},
{"adjacencies": [
{
"nodeFrom": "pid=6829@192.168.8.134",
"nodeTo": "pid=6733@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "star"
},
"id": "pid=6829@192.168.8.134",
"name": "condor_starter"
},
{"adjacencies": [
{
"nodeFrom": "pid=6809@192.168.8.134",
"nodeTo": "pid=6732@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "star"
},
"id": "pid=6809@192.168.8.134",
"name": "condor_starter"
},
{"adjacencies": [
{
"nodeFrom": "pid=6784@192.168.8.134",
"nodeTo": "pid=6732@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "star"
},
"id": "pid=6784@192.168.8.134",
"name": "condor_starter"
},
{"adjacencies": [
{
"nodeFrom": "pid=6764@192.168.8.134",
"nodeTo": "pid=6733@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FEE6CE",
"$type": "star"
},
"id": "pid=6764@192.168.8.134",
"name": "/usr/sbin/condor_starter"
},
{"adjacencies": [
{
"nodeFrom": "pid=6763@192.168.8.134",
"nodeTo": "pid=6732@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "star"
},
"id": "pid=6763@192.168.8.134",
"name": "condor_starter"
},
{"adjacencies": [
{
"nodeFrom": "pid=6834@192.168.8.134",
"nodeTo": "pid=6733@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "star"
},
"id": "pid=6834@192.168.8.134",
"name": "/bin/rm"
},
{"adjacencies": [
{
"nodeFrom": "pid=6722@192.168.8.134",
"nodeTo": "pid=6689@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "star"
},
"id": "pid=6722@192.168.8.134",
"name": "/usr/sbin/condor_master"
},
{"adjacencies": [
{
"nodeFrom": "pid=6814@192.168.8.134",
"nodeTo": "pid=6809@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FEE6CE",
"$type": "star"
},
"id": "pid=6814@192.168.8.134",
"name": "condor_starter"
},
{"adjacencies": [
{
"nodeFrom": "pid=6795@192.168.8.134",
"nodeTo": "pid=6732@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "star"
},
"id": "pid=6795@192.168.8.134",
"name": "condor_starter"
},
{"adjacencies": [
{
"nodeFrom": "pid=6751@192.168.8.134",
"nodeTo": "pid=6732@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "star"
},
"id": "pid=6751@192.168.8.134",
"name": "condor_starter"
},
{"adjacencies": [
{
"nodeFrom": "pid=6782@192.168.8.134",
"nodeTo": "pid=6733@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FEE6CE",
"$type": "star"
},
"id": "pid=6782@192.168.8.134",
"name": "kflops"
},
{"adjacencies": [
{
"nodeFrom": "pid=6820@192.168.8.134",
"nodeTo": "pid=6733@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "star"
},
"id": "pid=6820@192.168.8.134",
"name": "condor_procd"
},
{"adjacencies": [
{
"nodeFrom": "pid=6803@192.168.8.134",
"nodeTo": "pid=6795@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FEE6CE",
"$type": "star"
},
"id": "pid=6803@192.168.8.134",
"name": "condor_starter"
},
{"adjacencies": [
{
"nodeFrom": "pid=6770@192.168.8.134",
"nodeTo": "pid=6733@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FEE6CE",
"$type": "star"
},
"id": "pid=6770@192.168.8.134",
"name": "/usr/sbin/condor_starter.std"
},
{"adjacencies": [
{
"nodeTo": "pid=12299@192.168.8.133",
"nodeFrom": "pid=6732@192.168.8.134",
"data": {}
},
{
"nodeFrom": "pid=6732@192.168.8.134",
"nodeTo": "pid=1@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FEE6CE",
"$type": "star"
},
"id": "pid=6732@192.168.8.134",
"name": "/usr/sbin/condor_master"
},
{"adjacencies": [
{
"nodeFrom": "pid=6789@192.168.8.134",
"nodeTo": "pid=6784@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FEE6CE",
"$type": "star"
},
"id": "pid=6789@192.168.8.134",
"name": "condor_starter"
},
{"adjacencies": [
{
"nodeTo": "pid=12299@192.168.8.133",
"nodeFrom": "pid=6733@192.168.8.134",
"data": {}
},
{
"nodeTo": "pid=6732@192.168.8.134",
"nodeFrom": "pid=6733@192.168.8.134",
"data": {}
},
{
"nodeFrom": "pid=6733@192.168.8.134",
"nodeTo": "pid=6732@192.168.8.134",
"data": {"$color":"#FFEDA0"}
},
], 
"data": {
"$color": "#FC9272",
"$type": "star"
},
"id": "pid=6733@192.168.8.134",
"name": "condor_startd"
},
];





  // end
  // init ForceDirected
  var fd = new $jit.ForceDirected({
    //id of the visualization container
    injectInto: 'infovis',
    //Enable zooming and panning
    //with scrolling and DnD
    Navigation: {
      enable: true,
      type: 'Native',
      //Enable panning events only if we're dragging the empty
      //canvas (and not a node).
      panning: 'avoid nodes',
      zooming: 10 //zoom speed. higher is more sensible
    },
    // Change node and edge styles such as
    // color and width.
    // These properties are also set per node
    // with dollar prefixed data-properties in the
    // JSON structure.
    Node: {
      overridable: true,
      dim: 7
    },
    Edge: {
      overridable: true,
      color: '#23A4FF',
      lineWidth: 0.4
    },
    // Add node events
    Events: {
      enable: true,
      type: 'Native',
      //Change cursor style when hovering a node
      onMouseEnter: function() {
        fd.canvas.getElement().style.cursor = 'move';
      },
      onMouseLeave: function() {
        fd.canvas.getElement().style.cursor = '';
      },
      //Update node positions when dragged
      onDragMove: function(node, eventInfo, e) {
        var pos = eventInfo.getPos();
        node.pos.setc(pos.x, pos.y);
        fd.plot();
      },
      //Implement the same handler for touchscreens
      onTouchMove: function(node, eventInfo, e) {
        $jit.util.event.stop(e); //stop default touchmove event
        this.onDragMove(node, eventInfo, e);
      }
    },
    //Number of iterations for the FD algorithm
    iterations: 200,
    //Edge length
    levelDistance: 130,
    // This method is only triggered
    // on label creation and only for DOM labels (not native canvas ones).
    onCreateLabel: function(domElement, node){
      // Create a 'name' and 'close' buttons and add them
      // to the main node label
      var nameContainer = document.createElement('span'),
          closeButton = document.createElement('span'),
          style = nameContainer.style;
      nameContainer.className = 'name';
      nameContainer.innerHTML = node.name;
      closeButton.className = 'close';
      closeButton.innerHTML = 'x';
      domElement.appendChild(nameContainer);
      domElement.appendChild(closeButton);
      style.fontSize = "0.8em";
      style.color = "#ddd";
      //Fade the node and its connections when
      //clicking the close button
      closeButton.onclick = function() {
        node.setData('alpha', 0, 'end');
        node.eachAdjacency(function(adj) {
          adj.setData('alpha', 0, 'end');
        });
        fd.fx.animate({
          modes: ['node-property:alpha',
                  'edge-property:alpha'],
          duration: 500
        });
      };
      //Toggle a node selection when clicking
      //its name. This is done by animating some
      //node styles like its dimension and the color
      //and lineWidth of its adjacencies.
      nameContainer.onclick = function() {
        //set final styles
        fd.graph.eachNode(function(n) {
          if(n.id != node.id) delete n.selected;
          n.setData('dim', 7, 'end');
          n.eachAdjacency(function(adj) {
            adj.setDataset('end', {
              lineWidth: 0.4,
              color: '#23a4ff'
            });
          });
        });
        if(!node.selected) {
          node.selected = true;
          node.setData('dim', 17, 'end');
          node.eachAdjacency(function(adj) {
            adj.setDataset('end', {
              lineWidth: 3,
              color: '#36acfb'
            });
          });
        } else {
          delete node.selected;
        }
        //trigger animation to final styles
        fd.fx.animate({
          modes: ['node-property:dim',
                  'edge-property:lineWidth:color'],
          duration: 500
        });
        // Build the right column relations list.
        // This is done by traversing the clicked node connections.
        var html = "<h4>" + node.name + "</h4><b> connections:</b><ul><li>",
            list = [];
        node.eachAdjacency(function(adj){
          if(adj.getData('alpha')) list.push(adj.nodeTo.name);
        });
        //append connections information
        $jit.id('inner-details').innerHTML = html + list.join("</li><li>") + "</li></ul>";
      };
    },
    // Change node styles when DOM labels are placed
    // or moved.
    onPlaceLabel: function(domElement, node){
      var style = domElement.style;
      var left = parseInt(style.left);
      var top = parseInt(style.top);
      var w = domElement.offsetWidth;
      style.left = (left - w / 2) + 'px';
      style.top = (top + 10) + 'px';
      style.display = '';
    }
  });
  // load JSON data.
  fd.loadJSON(json);
  // compute positions incrementally and animate.
  fd.computeIncremental({
    iter: 40,
    property: 'end',
    onStep: function(perc){
      Log.write(perc + '% loaded...');
    },
    onComplete: function(){
      Log.write('done');
      fd.animate({
        modes: ['linear'],
        transition: $jit.Trans.Elastic.easeOut,
        duration: 2500
      });
    }
  });
  // end
}
