Drupal.behaviors.mathjax_wysiwyg_inline = function(context) { 
  // Remove the paragraphs around inline formulae
  $('span.formula.mathjax_inline').each(
    function() {
      var following = $(this).next().html();
      $(this).next().remove();
      $(this).appendTo($(this).prev());
      $(this).after(following);
    }
  );
};
