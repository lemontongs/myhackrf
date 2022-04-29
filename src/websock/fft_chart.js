
goog.require('proto.Packet');

var n_fft_size = 2048;
var fs = 2000000;

var fft_chart = new Chart(document.getElementById("fft-chart"), {
  type: 'line',
  data: {
    labels: linspace(-fs/2, fs/2, n_fft_size, false),
    datasets: [{ 
        data: Int16Array.from(Array(n_fft_size).keys()),
        pointRadius: 0,
        label: "FFT",
        borderColor: "#3e95cd",
        fill: false
      }
    ]
  },
  options: {
    title: {
      display: true,
      text: 'FFT'
    },
    animation: false,
    maintainAspectRatio: false,
    responsive: true
  }
});

var paused = false;
function pause()
{
  if (paused)
  {
    paused = false;
    send('more');
  }
  else
  {
    paused = true;
  }
}

function linspace(start, stop, num, endpoint = true)
{
  const div = endpoint ? (num - 1) : num;
  const step = (stop - start) / div;
  return Array.from({length: num}, (_, i) => start + step * i);
}

// Make a websocket connection
var connection = new WebSocket('ws://rpi4:8765')
connection.binaryType = "arraybuffer";

connection.onopen = function ()
{
  // Kick off a request for some IQ data
  connection.send('more');
};

// Log errors
connection.onerror = function (error)
{
  console.log('WebSocket Error ' + error);
};

// Log messages from the server
connection.onmessage = function (e)
{
  var message = proto.Packet.deserializeBinary(e.data);

  if (message.getHeader().getType() != proto.Packet_Header.PacketType.FFT)
    return;
  
  // Update charts
  fft_chart.data.datasets[0].data = message.getFftPacket().getFftList();
  fft_chart.data.labels = message.getFftPacket().getFreqBinsHzList();
  fft_chart.options.scales.y.max =  20;
  fft_chart.options.scales.y.min = -50;
  fft_chart.update();

  console.log('Got FFT');
  if (!paused)
  {
    connection.send('more'); // Ask for more data
  }
};
