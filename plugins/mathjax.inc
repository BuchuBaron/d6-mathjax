<?php
function wysiwyg_mathjax_mathjax_plugin() {
  $plugins = array();
  $plugins['mathjax'] = array(
    'title' => t('MathJax Integration'),
    'vendor url' => 'http://mathjax.com',
    'icon file' => 'add_math.png',
    'icon title' => t('Add MathML'),
    'settings' => array(
      'dialog' => array(
        'url' => '',
        'width' => 700,
        'height' => 300,
      ),
    ),
    'extended_valid_elements' => array('math', 'tex'),
  );
  return $plugins;
}
