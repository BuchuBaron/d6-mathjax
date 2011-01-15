<?php
/**
 * Implementation of hook_filter().
 */
function wysiwyg_mathjax_filter($op, $delta = 0, $format = -1, $text = '') {
  switch ($op) {
    case 'list':
      return array(0 => t('Drupal Wiki inline-formulae'));

    case 'description':
      return t('Converts Drupal Wiki inline formulae to HTML.');

    case 'process':
      return _wysiwyg_mathjax_filter_process($text);

    case 'no cache':
          return FALSE;

    case 'settings':
          $form = array();
          return $form;
    default:
      return $text;
  }
}

function _wysiwyg_mathjax_filter_pattern() {
  return '/\[\[wysiwyg_mathjax:(\d+):([^\]]*?)\]\]/e';
}

/**
 * Finds all the occurences of a inline image tag
 */
function _wysiwyg_mathjax_filter_process(&$body) {
  // Syntax: [[inlineformula:<iid>:<inlineargs>]]
  $body = preg_replace(array(_wysiwyg_mathjax_filter_pattern()), array("_wysiwyg_mathjax_render_nodeview($1,'$2');"), $body);
  return $body;
}

/**
 * returns a rendered(html) image for the tag
 */
function _wysiwyg_mathjax_render_nodeview($iid, $arguments) {
  $arguments = _wysiwyg_mathjax_unserialize_arguments(urldecode($arguments));
  $formula_obj = _wysiwyg_mathjax_load_inline_entity($iid);

  return theme('wysiwyg_mathjax_render_formula_entity', $formula_obj, $arguments);
}

/**
 * Loads the comma seperated argument list into an array
 */
function _wysiwyg_mathjax_unserialize_arguments($arguments) {
    // Argument syntax: key=value,key=value,key=value
    $result = array();
    // TODO: What about values with commas?
    $arguments = explode(',', $arguments);
    foreach ($arguments as $value) {
      $pair = explode('=', $value);
      $result[ $pair[0] ] = $pair[1];
    }
    return $result;
}
