Drupal.behaviors.wysiwyg_mathjax_uploadform = function () {
  $('#edit-wysiwyg-mathjax-file').bind('change', function() {
      $('.dialog-page-content *').hide();
      $('.dialog-page-content').addClass('uploading');
      $('form').submit();
    });
  $('#edit-wysiwyg-mathml-text').bind('change', function() {
      $('.dialog-page-content *').hide();
      $('.dialog-page-content').addClass('uploading');
      $('form').submit();
    });
}
