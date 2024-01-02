/*
Theme by: WebThemez.com
Note: Please use our back link in your site
*/
$(function () {
  $.vegas('slideshow', {
    delay: 3000,
    backgrounds: [
      { src: 'images/bg4.jpg', fade: 4000 },
      { src: 'images/bg3.jpg', fade: 4000 },
      { src: 'images/bg2.jpg', fade: 4000 },
      { src: 'images/bg1.jpg', fade: 4000 }
    ]
  })('overlay');

  var endDate = "December  24, 2023 12:00:00";

  $('.countdown.simple').countdown({ date: endDate });

  $('.countdown.styled').countdown({
    date: endDate,
    render: function (data) {
      $(this.el).html("<div>" + this.leadingZeros(data.days, 3) + " <span>days</span></div><div>" + this.leadingZeros(data.hours, 2) + " <span>hrs</span></div><div>" + this.leadingZeros(data.min, 2) + " <span>min</span></div><div>" + this.leadingZeros(data.sec, 2) + " <span>sec</span></div>");
    }
  });

  $('.countdown.callback').countdown({
    date: +(new Date) + 10000,
    render: function (data) {
      $(this.el).text(this.leadingZeros(data.sec, 2) + " sec");
    },
    onEnd: function () {
      $(this.el).addClass('ended');
    }
  }).on("click", function () {
    $(this).removeClass('ended').data('countdown').update(+(new Date) + 10000).start();
  });

  $('#frmLights').on('change', function() {
    $('#submit').click();
});

$(document).ready(function(){
  $('#frmLights').change(function(){
    // Call submit() method on <form id='myform'>
    $('#frmLights').submit();
  });


});

});


