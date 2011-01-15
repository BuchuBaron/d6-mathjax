<?php

/**
 * Providing a administration interface for tagging.
 */
function wysiwyg_mathjax_admin_settings() {
  $form['wysiwyg_mathjax_skip_dependencies_check'] = array(
    '#type' => 'checkbox',
    '#title' => t('Do the requirements checks'),
    '#description' => t('If this is set, requirements checks will be done. If something is not correct, you get an error message.'),
    '#default_value' => variable_get('wysiwyg_mathjax_skip_dependencies_check', TRUE),
  );

  $form['wysiwyg_mathjax_filename_as_title_default'] = array(
    '#type' => 'checkbox',
    '#title' => t('Filename as title'),
    '#description' => t('Should the filename be used as title by default. If not checked, the title will be empty and not shown by default'),
    '#default_value' => variable_get('wysiwyg_mathjax_filename_as_title_default', FALSE),
  );
  $form['styles'] = array(
    '#type' => 'fieldset',
    '#title' => t('Styles'),
    '#description' => t('Set the available styles (css) here'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
    '#weight' => -2,
  );
  $form['styles']['wysiwyg_mathjax_styles'] = array(
    '#type' => 'textarea',
    '#title' => t('Formula style'),
    '#description' => t('Define which CSS styles the user can chose when adding a formula. Please do not define floating or size in these CSS classes, as this is handled seperately. You can give the style a descriptive name to make it easier to handle. The syntax is "Descriptive Name=mathjax_style_XYC". You _must_ prefix the style with mathjax_style_*! For each style use on separate line.'),
    '#default_value' => variable_get('wysiwyg_mathjax_styles', '')
  );

  $form['paths']  = array(
    '#type' => 'fieldset',
    '#title' => t('Paths'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
    '#weight' => -2,
  );
  $form['wysiwyg_mathjax_max_filesize'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximal filesize in KB'),
    '#description' => t('Maximal filesize in KB for a MathML file, which gets uploaded. 0 for "no limitation"'),
    '#default_value' => variable_get('wysiwyg_mathjax_max_filesize', 0),
  );
  
  $form['paths']['wysiwyg_mathjax_destdir_group']['wysiwyg_mathjax_dest_root'] = array(
    '#type' => 'textfield',
    '#title' => t('Root destination'),
    '#description' => t('This is the root destination relative to the current files directory ("' . file_directory_path() . '"). Neither insert a preceding nor successive slash.'),
    '#default_value' => variable_get('wysiwyg_mathjax_dest_root', 'wysiwyg_mathjax')
  );

  $form['paths']['wysiwyg_mathjax_destdir_group']['wysiwyg_mathjax_relative_user'] = array(
    '#type' => 'textfield',
    '#title' => t('Subdirectory for private uploads'),
    '#description' => t('This is the user-files destination relative to the current wysiwyg_mathjax root destination(see above) directory (e.g. "' . file_directory_path() . '/rootdest/$uid"). $uid will be replaced by the current user-id. Keep blanc to save it in the root.'),
    '#default_value' => variable_get('wysiwyg_mathjax_relative_user', '$uid')
  );

  return system_settings_form($form);
}
