
goog.require('proto.Packet');

var buffer_size = 131072;
var n_fft_size = 2048;
var fs = 2000000;
var iq_chart_real_values = Int16Array.from(Array(buffer_size).keys());
var iq_chart_imag_values = Int16Array.from(Array(buffer_size).keys());
var mag_chart_y_values = Int16Array.from(Array(buffer_size).keys());
var fft_chart_y_values = Int16Array.from(Array(n_fft_size).keys());

var iq_chart = new Chart(document.getElementById("iq-chart"), {
  type: 'line',
  data: {
    labels: Array.from(Array(buffer_size).keys()),
    datasets: [{ 
        data: iq_chart_real_values,
        pointRadius: 0,
        label: "I",
        borderColor: "#3e95cd",
        fill: false
      }, { 
        data: iq_chart_imag_values,
        pointRadius: 0,
        label: "Q",
        borderColor: "#8e5ea2",
        fill: false
      }
    ]
  },
  options: {
    title: {
      display: true,
      text: 'IQ Data'
    },
    animation: false,
    maintainAspectRatio: false,
    responsive: true
  }
});

var mag_chart = new Chart(document.getElementById("mag-chart"), {
  type: 'line',
  data: {
    labels: Array.from(Array(buffer_size).keys()),
    datasets: [{ 
        data: mag_chart_y_values,
        pointRadius: 0,
        label: "Abs",
        borderColor: "#3e95cd",
        fill: false
      }
    ]
  },
  options: {
    title: {
      display: true,
      text: 'Magnitude'
    },
    animation: false,
    maintainAspectRatio: false,
    responsive: true
  }
});

var fft_chart = new Chart(document.getElementById("fft-chart"), {
  type: 'line',
  data: {
    labels: linspace(-fs/2/1e3, fs/2/1e3, n_fft_size, false),
    datasets: [{ 
        data: fft_chart_y_values,
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


// Create a Canvas element
const canvas = document.getElementById('waterfall-chart');
const canvasParent = document.getElementById('waterfall-chart').parentElement;
canvas.width = canvasParent.offsetWidth - 20;
canvas.height = canvasParent.offsetHeight - 20;
const ctx = canvas.getContext('2d');

var num_x_bins = (n_fft_size/16);
var bin_width = canvas.width / num_x_bins;
var num_y_bins = canvas.height / bin_width;
var bin_hight = canvas.height / num_y_bins;

// // Matrix
// var waterfall = math.ones(num_x_bins, num_y_bins);

// function redraw_waterfall()
// {
//   function fill_bin(bin_x, bin_y, color)
//   {
//     ctx.fillStyle = color;
//     ctx.fillRect(bin_x * bin_width, bin_y * bin_hight, bin_width, bin_hight);
//   };

//   waterfall.forEach(function (value, index, matrix) {
//     var value = Math.random()*255;
//     fill_bin(x, y, `rgb(0,0,${value})`);
//   });

//   for (var x = 0; x < num_x_bins; x++)
//   {
//     for (var y = 0; y < num_y_bins; y++)
//     {
//       var value = Math.random()*255;
//       fill_bin(x, y, `rgb(0,0,${value})`);
//     }
//   }
// }

// redraw_waterfall();


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

  if (message.getHeader().getType() != proto.Packet_Header.PacketType.IQ)
    return;
  
  var iq = message.getIqPacket().getSignalList();
  
  // Separate the real and imag parts
  let real = iq.filter((element, index) => {
      return index % 2 === 0;
  });
  let imag = iq.filter((element, index) => {
      return index % 2 === 1;
  });

  // Compute 20 log magnitude
  var mag = Array();
  for (var i=0; i<real.length; i++)
  {
    var r = 20.0 * Math.log10( Math.abs(Math.sqrt((real[i] * real[i]) + (imag[i] * imag[i]))) );
    if (Number.isNaN(r))
    {
      mag.push(0);
    }
    else
    {
      mag.push(r);
    }
  }

  // Update charts
  iq_chart_real_values.set(iq_chart_real_values.slice(real.length));
  iq_chart_real_values.set(real, iq_chart_real_values.length-real.length);
  
  iq_chart_imag_values.set(iq_chart_imag_values.slice(imag.length));
  iq_chart_imag_values.set(imag, iq_chart_imag_values.length-imag.length);
  
  iq_chart.update();

  
  mag_chart_y_values.set(mag_chart_y_values.slice(mag.length));
  mag_chart_y_values.set(mag, mag_chart_y_values.length-mag.length);
  
  mag_chart.update();

  // Normalize signal
  var real_normalized = Array(real.length);
  var imag_normalized = Array(imag.length);
  for (var i=0; i<real.length; i++)
  {
    real_normalized[i] = real[i] / 32768;
    imag_normalized[i] = imag[i] / 32768;
  }

  // Compute FFT
  fft = new FFTNayuki(n_fft_size);
  fft.forward(real_normalized, imag_normalized);

  // FFT shift and update chart
  for (var i=0; i<real_normalized.length; i++)
  {
    var fftshift_index = (i + (real_normalized.length/2)) % real_normalized.length;
    fft_chart_y_values[fftshift_index] = 20.0 * Math.log10( Math.abs(Math.sqrt((real_normalized[i] * real_normalized[i]) + (imag_normalized[i] * imag_normalized[i]))) );
  }
  fft_chart.update();

  console.log('Got ' + real.length + ' samples');
  if (!paused)
  {
    connection.send('more'); // Ask for more data
  }
};
