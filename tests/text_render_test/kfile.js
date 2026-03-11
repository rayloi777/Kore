let project = new Project('Text Render Test');

await project.addProject(findKore());

project.addFile('sources/**');
project.addFile('deployment/**');
project.setDebugDir('deployment');

project.addIncludeDir('../../sources/text');
project.addFile('../../sources/text/text.c');

project.addKongDir('shaders');

project.flatten();

resolve(project);
